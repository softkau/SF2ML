#include "preset_manager.hpp"

using namespace sflib;

PresetManager::PresetManager(const SfbkMap::Pdta& pdta) {
	if (!pdta.phdr || !pdta.pbag || !pdta.pmod || !pdta.pgen) {
		status = SFLIB_FAILED;
		return;
	}

	DWORD phdr_ck_size;
	std::memcpy(&phdr_ck_size, pdta.phdr + 4, sizeof(DWORD));

	size_t phdr_count = phdr_ck_size / sizeof(SfPresetHeader) - 1;
	for (DWORD i = 0; i < phdr_count; i++) {
		const BYTE* cur_ptr = pdta.phdr + 8 + i * sizeof(SfPresetHeader);
		SfPresetHeader cur, next;
		std::memcpy(&cur, cur_ptr, sizeof(cur));
		std::memcpy(&next, cur_ptr + sizeof(SfPresetHeader), sizeof(next));

		PresetData res {};
		memcpy(res.preset_name, cur.ach_preset_name, 20);
		res.preset_name[20] = 0;
		PresetIndex pid;
		pid.preset_number = cur.w_preset;
		pid.bank_number = cur.w_bank;

		const size_t bag_start = cur.w_preset_bag_ndx;
		const size_t bag_end = next.w_preset_bag_ndx;
		for (size_t bag_ndx = bag_start; bag_ndx < bag_end; bag_ndx++) {
			const BYTE* bag_ptr = pdta.pbag + 8 + bag_ndx * sizeof(SfPresetBag);
			SfPresetBag bag_cur, bag_next;
			std::memcpy(&bag_cur, bag_ptr, sizeof(SfPresetBag));
			std::memcpy(&bag_next, bag_ptr + sizeof(SfPresetBag), sizeof(SfPresetBag));
			
			// PMOD is skipped rn...
			const size_t gen_start = bag_cur.w_gen_ndx;
			const size_t gen_end = bag_next.w_gen_ndx;

			const BYTE* gen_ptr = pdta.igen + 8 + gen_start * sizeof(SfGenList);
			PresetZone zone(gen_ptr, gen_end - gen_start);
			
			if (zone.IsEmpty()) {
				continue;
			}
			
			if (bag_ndx == bag_start && !zone.HasGenerator(SfInstrument)) {
				res.global_zone = std::move(zone);
			} else if (zone.HasGenerator(SfInstrument)) {
				res.zones.push_back(std::move(zone));
			}
		}

		char name[21];
		std::strcpy(name, res.preset_name);
		SfHandle handle = presets.EmplaceBack(std::move(res));
		name_index.emplace(name, handle);
		pid_index.emplace(pid, handle);
	}

	status = SFLIB_SUCCESS;
}

DWORD PresetManager::ChunkSize() const {
	DWORD pdhr_ck_size = sizeof(ChunkHead) + (presets.Count() + 1) * sizeof(SfPresetHeader);
	DWORD pbag_ck_size = sizeof(ChunkHead);
	DWORD pmod_ck_size = sizeof(ChunkHead) + sizeof(SfModList); // no mod support
	DWORD pgen_ck_size = sizeof(ChunkHead);

	DWORD bag_count = 0;
	for (const PresetData& preset : presets) {
		if (preset.global_zone.has_value()) {
			bag_count++;
			pgen_ck_size += preset.global_zone->RequiredSize();
		}
		for (const auto& zone : preset.zones) {
			bag_count++;
			pgen_ck_size += zone.RequiredSize();
		}
	}
	
	pbag_ck_size += (bag_count + 1) * sizeof(SfPresetBag);
	pgen_ck_size += sizeof(SfGenList);

	return pdhr_ck_size + pbag_ck_size + pmod_ck_size + pgen_ck_size;
}

SflibError PresetManager::Serialize(BYTE *dst, const InstrumentManager& inst_manager, BYTE **end) const {
	BYTE* pos = dst;

	// serialize ./phdr
	BYTE* const phdr_head = pos;
	std::memcpy(phdr_head, "phdr", 4);
	DWORD phdr_ck_size = (presets.Count() + 1) * sizeof(SfInst);
	std::memcpy(phdr_head + 4, &phdr_ck_size, sizeof(DWORD));
	pos += 8;
	DWORD bag_idx = 0;
	for (const PresetData& preset : presets) {
		SfPresetHeader bits;
		std::memcpy(bits.ach_preset_name, preset.preset_name, 20);
		bits.w_preset_bag_ndx = bag_idx;
		std::memcpy(pos, &bits, sizeof(bits));
		bag_idx += preset.zones.size();
		if (preset.global_zone.has_value()) {
			bag_idx++;
		}
		pos += sizeof(bits);
	}
	SfPresetHeader eop { "EOP", bag_idx };
	std::memcpy(pos, &eop, sizeof(eop));
	pos += sizeof(eop);

	// serialize ./pbag
	BYTE* const pbag_head = pos;
	std::memcpy(pbag_head, "pbag", 4);
	DWORD pbag_ck_size = (presets.Count() + 1) * sizeof(SfPresetBag);
	std::memcpy(pbag_head + 4, &pbag_ck_size, sizeof(DWORD));
	pos += 8;
	DWORD gen_idx = 0;
	for (const PresetData& preset : presets) {
		if (preset.global_zone.has_value()) {
			SfPresetBag bits;
			bits.w_gen_ndx = gen_idx;
			bits.w_mod_ndx = 0;
			std::memcpy(pos, &bits, sizeof(bits));
			gen_idx += preset.global_zone->GeneratorCount();
			pos += sizeof(bits);
		};
		for (const auto& zone : preset.zones) {
			SfPresetBag bits;
			bits.w_gen_ndx = gen_idx;
			bits.w_mod_ndx = 0;
			std::memcpy(pos, &bits, sizeof(bits));
			gen_idx += zone.GeneratorCount();
			pos += sizeof(bits);
		}
	}
	SfPresetBag end_of_pbag { gen_idx, 0 };
	std::memcpy(pos, &end_of_pbag, sizeof(end_of_pbag));
	pos += sizeof(end_of_pbag);

	// serialize ./pmod
	BYTE* const pmod_head = pos;
	std::memcpy(pmod_head, "pmod", 4);
	DWORD pmod_ck_size = sizeof(SfModList);
	std::memcpy(pmod_head + 4, &pmod_ck_size, sizeof(DWORD));
	pos += 8;
	SfModList end_of_pmod {};
	std::memcpy(pos, &end_of_pmod, sizeof(end_of_pmod));
	pos += sizeof(end_of_pmod);

	// serialize ./pgen
	BYTE* const pgen_head = pos;
	std::memcpy(pgen_head, "pgen", 4);
	DWORD pgen_ck_size = (gen_idx + 1) * sizeof(SfGenList);
	std::memcpy(pgen_head + 4, &pgen_ck_size, sizeof(DWORD));
	pos += 8;
	for (const PresetData& preset : presets) {
		BYTE* next = pos;

		if (preset.global_zone.has_value()) {
			if (auto err = preset.global_zone->SerializeGenerators(pos, &next, inst_manager)) {
				if (end) {
					*end = next;
				}
				return err;
			}
			pos = next;
		}
		for (const auto& zone : preset.zones) {
			if (auto err = zone.SerializeGenerators(pos, &next, inst_manager)) {
				if (end) {
					*end = next;
				}
				return err;
			}
			pos = next;
		}
	}
	SfGenList end_of_pgen {};
	std::memcpy(pos, &end_of_pgen, sizeof(end_of_pgen));
	pos += sizeof(end_of_pgen);

	if (end) {
		*end = pos;
	}

	return SFLIB_SUCCESS;
}

SfHandle PresetManager::Add(PresetIndex pid, const std::string &name) {
	PresetData data {};
	std::memcpy(data.preset_name, name.c_str(), std::min<std::size_t>(20, name.length()));
	data.preset_name[20] = 0;

	SfHandle handle = presets.EmplaceBack(std::move(data));
	name_index.emplace(name, handle);
	pid_index.emplace(pid, handle);

	return handle;
}

SflibError PresetManager::Remove(SfHandle target) {
	if (presets.Remove(target)) {
		std::erase_if(name_index, [target](const auto& x) { return x.second == target; });
		std::erase_if(pid_index,  [target](const auto& x) { return x.second == target; });
		return SFLIB_SUCCESS;
	}
	return SFLIB_FAILED;
}

std::optional<SfHandle> PresetManager::FindPreset(const std::string& name) const {
	if (auto it = name_index.find(name); it != name_index.end()) {
		return it->second;
	}
	return std::nullopt;
}

std::optional<SfHandle> PresetManager::FindPreset(PresetIndex pid) const {
	if (auto it = pid_index.find(pid); it != pid_index.end()) {
		return it->second;
	}
	return std::nullopt;
}

auto PresetManager::FindPresets(const std::string &name) const
	-> std::optional<std::pair<IndexContainer1::const_iterator, IndexContainer1::const_iterator>>
{
	auto res = name_index.equal_range(name);
	if (res.first != res.second) {
		return res;
	}
	return std::nullopt;
}

auto PresetManager::FindPresets(PresetIndex pid) const
	-> std::optional<std::pair<IndexContainer2::const_iterator, IndexContainer2::const_iterator>>
{
	auto res = pid_index.equal_range(pid);
	if (res.first != res.second) {
		return res;
	}
	return std::nullopt;
}

