#include "instrument_manager.hpp"

#include <set>

using namespace sflib;

InstrumentManager::InstrumentManager(const SfbkMap::Pdta& pdta) {
	if (!pdta.inst || !pdta.ibag || !pdta.imod || !pdta.igen) {
		status = SFLIB_FAILED;
		return;
	}

	// validate chunks (...skipped)

	DWORD inst_ck_size;
	std::memcpy(&inst_ck_size, pdta.inst + 4, sizeof(DWORD));

	size_t inst_count = inst_ck_size / sizeof(SfInst) - 1;
	for (InstID id = 0; id < inst_count; id++) {
		const BYTE* cur_ptr = pdta.inst + 8 + id * sizeof(SfInst);
		SfInst cur, next;
		std::memcpy(&cur, cur_ptr, sizeof(SfInst));
		std::memcpy(&next, cur_ptr + sizeof(SfInst), sizeof(SfInst));

		InstData res {};
		memcpy(res.inst_name, cur.ach_inst_name, 20);
		res.inst_name[20] = 0;

		const size_t bag_start = cur.w_inst_bag_ndx;
		const size_t bag_end = next.w_inst_bag_ndx;
		for (size_t bag_ndx = bag_start; bag_ndx < bag_end; bag_ndx++) {
			const BYTE* bag_ptr = pdta.ibag + 8 + bag_ndx * sizeof(SfInstBag);
			SfInstBag bag_cur, bag_next;
			std::memcpy(&bag_cur, bag_ptr, sizeof(SfInstBag));
			std::memcpy(&bag_next, bag_ptr + sizeof(SfInstBag), sizeof(SfInstBag));
			
			// IMOD is skipped rn...
			const size_t gen_start = bag_cur.w_inst_gen_ndx;
			const size_t gen_end = bag_next.w_inst_gen_ndx;

			const BYTE* gen_ptr = pdta.igen + 8 + gen_start * sizeof(SfInstGenList);
			InstZone zone(gen_ptr, gen_end - gen_start);

			if (zone.IsEmpty()) {
				continue;
			}
			
			if (bag_ndx == bag_start && !zone.HasGenerator(SfSampleID)) {
				res.global_zone = std::move(zone);
			} else if (zone.HasGenerator(SfSampleID)) {
				res.zones.push_back(std::move(zone));
			}
		}

		char name[21];
		std::strcpy(name, res.inst_name);
		SfHandle handle = insts.EmplaceBack(std::move(res));
		inst_index.emplace(name, handle);
	}
	status = SFLIB_SUCCESS;
}

DWORD InstrumentManager::ChunkSize() const {
	DWORD inst_ck_size = sizeof(ChunkHead) + (insts.Count() + 1) * sizeof(SfInst);
	DWORD ibag_ck_size = sizeof(ChunkHead);
	DWORD imod_ck_size = sizeof(ChunkHead) + sizeof(SfInstModList); // no mod support
	DWORD igen_ck_size = sizeof(ChunkHead);

	DWORD bag_count = 0;
	for (const InstData& inst : insts) {
		if (inst.global_zone.has_value()) {
			bag_count++;
			igen_ck_size += inst.global_zone->RequiredSize();
		}
		for (const auto& zone : inst.zones) {
			bag_count++;
			igen_ck_size += zone.RequiredSize();
		}
	}
	
	ibag_ck_size += (bag_count + 1) * sizeof(SfInstBag);
	igen_ck_size += sizeof(SfInstGenList);

	return inst_ck_size + ibag_ck_size + imod_ck_size + igen_ck_size;
}

SflibError InstrumentManager::Serialize(BYTE* dst, const SampleManager& sample_manager, BYTE** end) const {
	BYTE* pos = dst;

	// serialize ./inst
	BYTE* const inst_head = pos;
	std::memcpy(inst_head, "inst", 4);
	DWORD inst_ck_size = (insts.Count() + 1) * sizeof(SfInst);
	std::memcpy(inst_head + 4, &inst_ck_size, sizeof(DWORD));
	pos += 8;
	DWORD bag_idx = 0;
	for (const InstData& inst : insts) {
		SfInst bits;
		std::memcpy(bits.ach_inst_name, inst.inst_name, 20);
		bits.w_inst_bag_ndx = bag_idx;
		std::memcpy(pos, &bits, sizeof(bits));
		bag_idx += inst.zones.size();
		if (inst.global_zone.has_value()) {
			bag_idx++;
		}
		pos += sizeof(bits);
	}
	SfInst eoi { "EOI", bag_idx };
	std::memcpy(pos, &eoi, sizeof(eoi));
	pos += sizeof(eoi);

	// serialize ./ibag
	BYTE* const ibag_head = pos;
	std::memcpy(ibag_head, "ibag", 4);
	DWORD ibag_ck_size = (insts.Count() + 1) * sizeof(SfInstBag);
	std::memcpy(ibag_head + 4, &ibag_ck_size, sizeof(DWORD));
	pos += 8;
	DWORD gen_idx = 0;
	for (const InstData& inst : insts) {
		if (inst.global_zone.has_value()) {
			SfInstBag bits;
			bits.w_inst_gen_ndx = gen_idx;
			bits.w_inst_mod_ndx = 0;
			std::memcpy(pos, &bits, sizeof(bits));
			gen_idx += inst.global_zone->GeneratorCount();
			pos += sizeof(bits);
		};
		for (const auto& zone : inst.zones) {
			SfInstBag bits;
			bits.w_inst_gen_ndx = gen_idx;
			bits.w_inst_mod_ndx = 0;
			std::memcpy(pos, &bits, sizeof(bits));
			gen_idx += zone.GeneratorCount();
			pos += sizeof(bits);
		}
	}
	SfInstBag end_of_ibag { gen_idx, 0 };
	std::memcpy(pos, &end_of_ibag, sizeof(end_of_ibag));
	pos += sizeof(end_of_ibag);

	// serialize ./imod
	BYTE* const imod_head = pos;
	std::memcpy(imod_head, "imod", 4);
	DWORD imod_ck_size = sizeof(SfInstModList);
	std::memcpy(imod_head + 4, &imod_ck_size, sizeof(DWORD));
	pos += 8;
	SfInstModList end_of_imod {};
	std::memcpy(pos, &end_of_imod, sizeof(end_of_imod));
	pos += sizeof(end_of_imod);

	// serialize ./igen
	BYTE* const igen_head = pos;
	std::memcpy(igen_head, "igen", 4);
	DWORD igen_ck_size = (gen_idx + 1) * sizeof(SfInstGenList);
	std::memcpy(igen_head + 4, &igen_ck_size, sizeof(DWORD));
	pos += 8;
	for (const InstData& inst : insts) {
		BYTE* next = pos;

		if (inst.global_zone.has_value()) {
			if (auto err = inst.global_zone->SerializeGenerators(pos, &next, sample_manager)) {
				if (end) {
					*end = next;
				}
				return err;
			}
			pos = next;
		}
		for (const auto& zone : inst.zones) {
			if (auto err = zone.SerializeGenerators(pos, &next, sample_manager)) {
				if (end) {
					*end = next;
				}
				return err;
			}
			pos = next;
		}
	}
	SfInstGenList end_of_igen {};
	std::memcpy(pos, &end_of_igen, sizeof(end_of_igen));
	pos += sizeof(end_of_igen);

	if (end) {
		*end = pos;
	}

	return SFLIB_SUCCESS;
}

SfHandle InstrumentManager::Add(const std::string& name) {
	InstData data;
	std::memcpy(data.inst_name, name.c_str(), std::min<std::size_t>(20, name.length()));
	data.inst_name[20] = 0;

	SfHandle handle = insts.EmplaceBack(std::move(data));
	inst_index.emplace(name, handle);

	return handle;
}

SflibError InstrumentManager::Remove(SfHandle target) {
	if (insts.Remove(target)) {
		std::erase_if(inst_index, [target](const auto& x) { return x.second == target; });
		return SFLIB_SUCCESS;
	}
	return SFLIB_FAILED;
}

std::optional<InstID> InstrumentManager::GetInstID(SfHandle target) const {
	return insts.GetID(target);
}

std::optional<SfHandle> InstrumentManager::FindInst(const std::string &name) const
{
	if (auto it = inst_index.find(name); it != inst_index.end()) {
		return it->second;
	}
	return std::nullopt;
}

auto InstrumentManager::FindInsts(const std::string& name) const
	-> std::optional<std::pair<IndexContainer::const_iterator, IndexContainer::const_iterator>>
{
	auto res = inst_index.equal_range(name);
	if (res.first != res.second) {
		return res;
	}
	return std::nullopt;
}
