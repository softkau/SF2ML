#include "instrument_manager.hpp"
#include "sample_manager.hpp"

#include <set>

using namespace sflib;

DWORD InstrumentManager::ChunkSize() const {
	DWORD inst_ck_size = sizeof(ChunkHead) + (insts.Count() + 1) * sizeof(spec::SfInst);
	DWORD ibag_ck_size = sizeof(ChunkHead);
	DWORD imod_ck_size = sizeof(ChunkHead) + sizeof(spec::SfInstModList); // no mod support
	DWORD igen_ck_size = sizeof(ChunkHead);

	DWORD bag_count = 0;
	for (const SfInstrument& inst : insts) {
		for (const auto& zone : inst.zones) {
			if (zone.IsEmpty() == false) {
				bag_count++;
				igen_ck_size += zone.RequiredSize();
			}
		}
	}
	
	ibag_ck_size += (bag_count + 1) * sizeof(spec::SfInstBag);
	igen_ck_size += sizeof(spec::SfInstGenList);

	return inst_ck_size + ibag_ck_size + imod_ck_size + igen_ck_size;
}

SflibError InstrumentManager::Serialize(BYTE* dst, BYTE** end) const {
	BYTE* pos = dst;

	// serialize ./inst
	BYTE* const inst_head = pos;
	std::memcpy(inst_head, "inst", 4);
	DWORD inst_ck_size = (insts.Count() + 1) * sizeof(spec::SfInst);
	std::memcpy(inst_head + 4, &inst_ck_size, sizeof(DWORD));
	pos += 8;
	DWORD bag_idx = 0;
	for (const SfInstrument& inst : insts) {
		spec::SfInst bits;
		std::memcpy(bits.ach_inst_name, inst.inst_name, 20);
		bits.w_inst_bag_ndx = bag_idx;
		std::memcpy(pos, &bits, sizeof(bits));
		bag_idx += inst.zones.CountIf([](const SfInstrumentZone& z) { return !z.IsEmpty(); });
		pos += sizeof(bits);
	}
	spec::SfInst eoi { "EOI", bag_idx };
	std::memcpy(pos, &eoi, sizeof(eoi));
	pos += sizeof(eoi);

	// serialize ./ibag
	BYTE* const ibag_head = pos;
	std::memcpy(ibag_head, "ibag", 4);
	DWORD ibag_ck_size = (bag_idx + 1) * sizeof(spec::SfInstBag);
	std::memcpy(ibag_head + 4, &ibag_ck_size, sizeof(DWORD));
	pos += 8;
	DWORD gen_idx = 0;
	for (const SfInstrument& inst : insts) {
		for (const auto& zone : inst.zones) {
			if (zone.IsEmpty()) {
				continue;
			}
			spec::SfInstBag bits;
			bits.w_inst_gen_ndx = gen_idx;
			bits.w_inst_mod_ndx = 0;
			std::memcpy(pos, &bits, sizeof(bits));
			gen_idx += zone.GeneratorCount();
			pos += sizeof(bits);
		}
	}
	spec::SfInstBag end_of_ibag { gen_idx, 0 };
	std::memcpy(pos, &end_of_ibag, sizeof(end_of_ibag));
	pos += sizeof(end_of_ibag);

	// serialize ./imod
	BYTE* const imod_head = pos;
	std::memcpy(imod_head, "imod", 4);
	DWORD imod_ck_size = sizeof(spec::SfInstModList);
	std::memcpy(imod_head + 4, &imod_ck_size, sizeof(DWORD));
	pos += 8;
	spec::SfInstModList end_of_imod {};
	std::memcpy(pos, &end_of_imod, sizeof(end_of_imod));
	pos += sizeof(end_of_imod);

	// serialize ./igen
	BYTE* const igen_head = pos;
	std::memcpy(igen_head, "igen", 4);
	DWORD igen_ck_size = (gen_idx + 1) * sizeof(spec::SfInstGenList);
	std::memcpy(igen_head + 4, &igen_ck_size, sizeof(DWORD));
	pos += 8;
	for (const SfInstrument& inst : insts) {
		BYTE* next = pos;
		for (const auto& zone : inst.zones) {
			if (zone.IsEmpty()) {
				continue;
			}
			
			if (auto err = zone.SerializeGenerators(pos, &next, sample_manager)) {
				if (end) {
					*end = next;
				}
				return err;
			}
			pos = next;
		}
	}
	spec::SfInstGenList end_of_igen {};
	std::memcpy(pos, &end_of_igen, sizeof(end_of_igen));
	pos += sizeof(end_of_igen);

	if (end) {
		*end = pos;
	}

	return SFLIB_SUCCESS;
}

SfInstrument& InstrumentManager::NewInstrument(const std::string& name) {
	SfInstrument& data = insts.NewItem();
	std::memcpy(data.inst_name, name.c_str(), std::min<std::size_t>(20, name.length()));
	data.inst_name[20] = 0;

	return data;
}

void InstrumentManager::Remove(SfHandle target, RemovalMode rm_mode) {
	auto* inst = insts.Get(target);
	if (inst) {
		auto zones = inst->FindZones([](const SfInstrumentZone& x) { return x.HasGenerator(SfGenSampleID); });
		for (SfHandle zone : zones) {
			SfHandle smpl_hand = *inst->GetZone(zone).GetSampleHandle();
			SfSample* smpl = sample_manager.Get(smpl_hand);
			if (!smpl) { // sample has been deleted
				continue;
			}
			auto it = smpl_ref_count.find(smpl_hand);
			it->second--;

			if (rm_mode == RemovalMode::Recursive) {
				auto linked_smpl_hand = smpl->GetLink();

				int ref_count = 0;
				ref_count += it->second;

				if (linked_smpl_hand) {
					auto it = smpl_ref_count.find(*linked_smpl_hand);
					ref_count += it->second;
				}
				assert(ref_count >= 0);
				if (ref_count == 0) {
					sample_manager.Remove(smpl_hand, RemovalMode::Recursive);
				}
			}
		}
		insts.Remove(target);
	}
}

std::optional<InstID> InstrumentManager::GetInstID(SfHandle target) const {
	return insts.GetID(target);
}
