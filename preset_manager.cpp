#include "preset_manager.hpp"

using namespace sflib;

DWORD PresetManager::ChunkSize() const {
	DWORD pdhr_ck_size = sizeof(ChunkHead) + (presets.Count() + 1) * sizeof(spec::SfPresetHeader);
	DWORD pbag_ck_size = sizeof(ChunkHead);
	DWORD pmod_ck_size = sizeof(ChunkHead) + sizeof(spec::SfModList); // no mod support
	DWORD pgen_ck_size = sizeof(ChunkHead);

	DWORD bag_count = 0;
	for (const SfPreset& preset : presets) {
		for (const auto& zone : preset.zones) {
			if (zone.IsEmpty()) {
				continue;
			}
			bag_count++;
			pgen_ck_size += zone.RequiredSize();
		}
	}
	
	pbag_ck_size += (bag_count + 1) * sizeof(spec::SfPresetBag);
	pgen_ck_size += sizeof(spec::SfGenList);

	return pdhr_ck_size + pbag_ck_size + pmod_ck_size + pgen_ck_size;
}

SflibError PresetManager::Serialize(BYTE* dst, BYTE** end) const {
	BYTE* pos = dst;

	// serialize ./phdr
	BYTE* const phdr_head = pos;
	std::memcpy(phdr_head, "phdr", 4);
	DWORD phdr_ck_size = (presets.Count() + 1) * sizeof(spec::SfPresetHeader);
	std::memcpy(phdr_head + 4, &phdr_ck_size, sizeof(DWORD));
	pos += 8;
	DWORD bag_idx = 0;
	for (const SfPreset& preset : presets) {
		spec::SfPresetHeader bits {};
		std::memcpy(bits.ach_preset_name, preset.preset_name, 20);
		bits.w_preset_bag_ndx = bag_idx;
		bits.w_preset = preset.preset_number;
		bits.w_bank = preset.bank_number;
		std::memcpy(pos, &bits, sizeof(bits));
		bag_idx += preset.zones.CountIf([](const SfPresetZone& zone) { return zone.IsEmpty() == false; });
		pos += sizeof(bits);
	}
	spec::SfPresetHeader eop { "EOP" };
	eop.w_preset_bag_ndx = bag_idx;
	std::memcpy(pos, &eop, sizeof(eop));
	pos += sizeof(eop);

	// serialize ./pbag
	BYTE* const pbag_head = pos;
	std::memcpy(pbag_head, "pbag", 4);
	DWORD pbag_ck_size = (bag_idx + 1) * sizeof(spec::SfPresetBag);
	std::memcpy(pbag_head + 4, &pbag_ck_size, sizeof(DWORD));
	pos += 8;
	DWORD gen_idx = 0;
	for (const SfPreset& preset : presets) {
		for (const auto& zone : preset.zones) {
			if (zone.IsEmpty()) {
				continue;
			}
			spec::SfPresetBag bits;
			bits.w_gen_ndx = gen_idx;
			bits.w_mod_ndx = 0;
			std::memcpy(pos, &bits, sizeof(bits));
			gen_idx += zone.GeneratorCount();
			pos += sizeof(bits);
		}
	}
	spec::SfPresetBag end_of_pbag { gen_idx, 0 };
	std::memcpy(pos, &end_of_pbag, sizeof(end_of_pbag));
	pos += sizeof(end_of_pbag);

	// serialize ./pmod
	BYTE* const pmod_head = pos;
	std::memcpy(pmod_head, "pmod", 4);
	DWORD pmod_ck_size = sizeof(spec::SfModList);
	std::memcpy(pmod_head + 4, &pmod_ck_size, sizeof(DWORD));
	pos += 8;
	spec::SfModList end_of_pmod {};
	std::memcpy(pos, &end_of_pmod, sizeof(end_of_pmod));
	pos += sizeof(end_of_pmod);

	// serialize ./pgen
	BYTE* const pgen_head = pos;
	std::memcpy(pgen_head, "pgen", 4);
	DWORD pgen_ck_size = (gen_idx + 1) * sizeof(spec::SfGenList);
	std::memcpy(pgen_head + 4, &pgen_ck_size, sizeof(DWORD));
	pos += 8;
	for (const SfPreset& preset : presets) {
		BYTE* next = pos;
		for (const auto& zone : preset.zones) {
			if (zone.IsEmpty()) {
				continue;
			}

			if (auto err = zone.SerializeGenerators(pos, &next, instrument_manager)) {
				if (end) {
					*end = next;
				}
				return err;
			}
			pos = next;
		}
	}
	spec::SfGenList end_of_pgen {};
	std::memcpy(pos, &end_of_pgen, sizeof(end_of_pgen));
	pos += sizeof(end_of_pgen);

	if (end) {
		*end = pos;
	}

	return SFLIB_SUCCESS;
}

SfPreset& PresetManager::NewPreset(std::uint16_t preset_number,
								   std::uint16_t bank_number,
								   const std::string& name) {
	SfPreset& rec = presets.NewItem();
	std::memcpy(rec.preset_name, name.c_str(), std::min<std::size_t>(20, name.length()));
	rec.preset_name[20] = 0;
	rec.preset_number = preset_number;
	rec.bank_number = bank_number;

	return rec;
}

void PresetManager::Remove(SfHandle target) {
	presets.Remove(target);
}