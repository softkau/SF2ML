#include "sfloader.hpp"
#include <sfgenerator.hpp>
#include <tuple>
#include <map>
#include <set>

namespace {
	SF2ML::SfGenAmount InterpretGenerator(SF2ML::spec::SfGenList gen_entry) {
		using namespace SF2ML;
		switch (gen_entry.sf_gen_oper) {
			case SfGenKeyRange:
			case SfGenVelRange: {
				Ranges<BYTE> res = {
					gen_entry.gen_amount.ranges.by_lo,
					gen_entry.gen_amount.ranges.by_hi
				};
				return SfGenAmount{res};
			}
			case SfGenSampleID: {
				SmplHandle res(gen_entry.gen_amount.w_amount);
				return SfGenAmount{res};
			}
			case SfGenInstrument: {
				InstHandle res(gen_entry.gen_amount.w_amount);
				return SfGenAmount{res};
			}
			case SfGenSampleModes: {
				WORD w_amount = gen_entry.gen_amount.w_amount;
				return SfGenAmount{w_amount};
			}
			default: {
				SHORT sh_amount = gen_entry.gen_amount.sh_amount;
				return SfGenAmount{sh_amount};
			}
		}
	}

	SF2ML::SfGenAmount InterpretGenerator(SF2ML::spec::SfInstGenList gen_entry) {
		using namespace SF2ML;
		switch (gen_entry.sf_gen_oper) {
			case SfGenKeyRange:
			case SfGenVelRange: {
				Ranges<BYTE> res = {
					gen_entry.gen_amount.ranges.by_lo,
					gen_entry.gen_amount.ranges.by_hi
				};
				return SfGenAmount{res};
			}
			case SfGenSampleID: {
				SmplHandle res(gen_entry.gen_amount.w_amount);
				return SfGenAmount{res};
			}
			case SfGenInstrument: {
				InstHandle res(gen_entry.gen_amount.w_amount);
				return SfGenAmount{res};
			}
			case SfGenSampleModes: {
				WORD w_amount = gen_entry.gen_amount.w_amount;
				return SfGenAmount{w_amount};
			}
			default: {
				SHORT sh_amount = gen_entry.gen_amount.sh_amount;
				return SfGenAmount{sh_amount};
			}
		}
	}

	namespace recursive {
		std::set<SF2ML::WORD> active;
		std::set<SF2ML::WORD> visited;
		std::map<SF2ML::WORD, bool> illegal;
		bool DFSModulators(SF2ML::WORD mod_ndx, const SF2ML::BYTE* buf, SF2ML::DWORD count) {
			if (auto it = visited.find(mod_ndx); it != visited.end()) { // previously visited modulator
				auto it2 = illegal.find(mod_ndx);
				if (it2 == illegal.end()) { // cyclic link found
					illegal.emplace(mod_ndx, true);
					return true;
				} else { // the modulator has already been finished testing
					return it2->second;
				}
			}

			visited.insert(mod_ndx);
			SF2ML::spec::SfInstModList mod;
			std::memcpy(&mod, buf + mod_ndx * sizeof(mod), sizeof(mod));

			if (mod.sf_mod_dest_oper & 0x8000) { // link to another modulator
				SF2ML::WORD link_ndx = mod.sf_mod_dest_oper & 0x7FFF;

				if (link_ndx < count && active.find(link_ndx) != active.end()) { // in bounds link
					bool r = DFSModulators(mod_ndx, buf, count);
					illegal.emplace(mod_ndx, r);
					return r;
				} else { // out of bounds link
					illegal.emplace(mod_ndx, true);
					return true;
				}
			} else { // destination is generator (no cyclic link found)
				illegal.emplace(mod_ndx, false);
				return false;
			}
		}

		void InitDFS(const std::map<std::tuple<SF2ML::SFModulator,
											   SF2ML::SFGenerator,
											   SF2ML::SFModulator>,
					 SF2ML::WORD>& actives) {
			visited.clear();
			illegal.clear();
			active.clear();
			for (const auto& [mod_op, mod_ndx] : actives) {
				active.insert(mod_ndx);
			}
		}
	}
	
}

auto SF2ML::loader::LoadSfbk(SfInfo& infos,
							 PresetContainer& presets,
							 InstContainer& insts,
							 SmplContainer& smpls,
							 const SfbkMap& sfbk)
							 -> SF2ML::SF2MLError {
	
	if (auto err = LoadInfos(infos, sfbk)) {
		return err;
	}
	if (auto err = LoadSamples(smpls, sfbk)) {
		return err;
	}
	if (auto err = LoadPresets(presets, sfbk)) {
		return err;
	}
	if (auto err = LoadInstruments(insts, sfbk)) {
		return err;
	}
	
	return SF2ML_SUCCESS;
}

auto SF2ML::loader::LoadInfos(SfInfo& infos, const SfbkMap& sfbk) -> SF2ML::SF2MLError
{
	const auto& info = sfbk.info;

	ChunkHead ck_head;
	auto validate_zstr = [&ck_head](const BYTE* ck_data) -> bool {
		return ck_data[ck_head.ck_size - 1] == 0x00;
	};

	// ./ifil
	std::memcpy(&ck_head, info.ifil, sizeof(ck_head));
	if (ck_head.ck_size != 4) {
		return SF2ML_INVALID_CK_SIZE;
	}
	spec::SfVersionTag ver;
	std::memcpy(&ver, info.ifil + 8, sizeof(ver));
	infos.SetSoundFontVersion({
		.major = ver.w_major,
		.minor = ver.w_minor
	});

	// ./isng
	std::memcpy(&ck_head, info.isng, sizeof(ck_head));
	if (validate_zstr(info.isng + 8)) {
		infos.SetSoundEngine(reinterpret_cast<const char*>(info.isng + 8));
	} else {
		infos.SetSoundEngine("EMU8000");
	}

	// ./inam
	std::memcpy(&ck_head, info.inam, sizeof(ck_head));
	if (validate_zstr(info.inam + 8)) {
		infos.SetBankName(reinterpret_cast<const char*>(info.inam + 8));
	} else {
		return SF2ML_ZSTR_CHECK_FAILED;
	}

	if (info.irom && validate_zstr(info.irom)) {
		infos.SetSoundRomName(reinterpret_cast<const char*>(info.irom + 8));
	}
	if (info.iver) {
		std::memcpy(&ck_head, info.iver, sizeof(ck_head));
		if (ck_head.ck_size == 4) {
			spec::SfVersionTag ver;
			std::memcpy(&ver, info.iver + 8, sizeof(ver));
			infos.SetSoundRomVersion(VersionTag{
				.major = ver.w_major,
				.minor = ver.w_minor
			});
		}
	}
	if (info.icrd && validate_zstr(info.icrd)) {
		infos.SetSoundRomName(reinterpret_cast<const char*>(info.icrd + 8));
	}
	if (info.ieng && validate_zstr(info.ieng)) {
		infos.SetAuthor(reinterpret_cast<const char*>(info.ieng + 8));
	}
	if (info.iprd && validate_zstr(info.iprd)) {
		infos.SetTargetProduct(reinterpret_cast<const char*>(info.iprd + 8));
	}
	if (info.icop && validate_zstr(info.icop)) {
		infos.SetCopyrightMessage(reinterpret_cast<const char*>(info.icop + 8));
	}
	if (info.icmt && validate_zstr(info.icmt)) {
		infos.SetComments(reinterpret_cast<const char*>(info.icmt + 8));
	}
	if (info.isft && validate_zstr(info.isft)) {
		infos.SetToolUsed(reinterpret_cast<const char*>(info.isft + 8));
	}

	return SF2ML_SUCCESS;
}

auto SF2ML::loader::LoadPresets(PresetContainer& presets, const SfbkMap& sfbk) -> SF2ML::SF2MLError {
	const auto& pdta = sfbk.pdta;

	DWORD phdr_ck_size;
	std::memcpy(&phdr_ck_size, pdta.phdr + 4, sizeof(DWORD));

	size_t phdr_count = phdr_ck_size / sizeof(spec::SfPresetHeader) - 1;
	for (DWORD i = 0; i < phdr_count; i++) {
		const BYTE* cur_ptr = pdta.phdr + 8 + i * sizeof(spec::SfPresetHeader);
		spec::SfPresetHeader cur, next;
		std::memcpy(&cur, cur_ptr, sizeof(cur));
		std::memcpy(&next, cur_ptr + sizeof(spec::SfPresetHeader), sizeof(next));

		SfPreset& rec = presets.NewItem();
		rec.SetName(std::string(reinterpret_cast<const char*>(cur.ach_preset_name), 20));
		rec.SetPresetNumber(cur.w_preset);
		rec.SetBankNumber(cur.w_bank);

		const size_t bag_start = cur.w_preset_bag_ndx;
		const size_t bag_end = next.w_preset_bag_ndx;
		for (size_t bag_ndx = bag_start; bag_ndx < bag_end; bag_ndx++) {
			const BYTE* bag_ptr = pdta.pbag + 8 + bag_ndx * sizeof(spec::SfPresetBag);
			spec::SfPresetBag bag_cur, bag_next;
			std::memcpy(&bag_cur, bag_ptr, sizeof(spec::SfPresetBag));
			std::memcpy(&bag_next, bag_ptr + sizeof(spec::SfPresetBag), sizeof(spec::SfPresetBag));

			// PMOD is skipped rn...
			const size_t gen_start = bag_cur.w_gen_ndx;
			const size_t gen_end = bag_next.w_gen_ndx;

			const BYTE* gen_ptr = pdta.pgen + 8 + gen_start * sizeof(spec::SfGenList);
			
			SfPresetZone zone(PZoneHandle(0));
			if (auto err = LoadGenerators(zone, gen_ptr, gen_end - gen_start)) {
				return err;
			}

			if (!zone.IsEmpty()) {
				if (bag_ndx == bag_start && !zone.HasGenerator(SfGenInstrument)) {
					rec.GetGlobalZone().MoveProperties(std::move(zone));
				} else if (zone.HasGenerator(SfGenInstrument)) {
					rec.NewZone().MoveProperties(std::move(zone));
				}
			}
		}
	}

	return SF2ML_SUCCESS;
}

auto SF2ML::loader::LoadInstruments(InstContainer& insts, const SfbkMap& sfbk) -> SF2ML::SF2MLError {
	const auto& pdta = sfbk.pdta;

		DWORD inst_ck_size;
		std::memcpy(&inst_ck_size, pdta.inst + 4, sizeof(DWORD));

		size_t inst_count = inst_ck_size / sizeof(spec::SfInst) - 1;
		for (DWORD id = 0; id < inst_count; id++) {
			const BYTE* cur_ptr = pdta.inst + 8 + id * sizeof(spec::SfInst);
			spec::SfInst cur, next;
			std::memcpy(&cur, cur_ptr, sizeof(spec::SfInst));
			std::memcpy(&next, cur_ptr + sizeof(spec::SfInst), sizeof(spec::SfInst));

			SfInstrument& rec = insts.NewItem();
			rec.SetName(std::string(reinterpret_cast<const char*>(cur.ach_inst_name), 20));

			const size_t bag_start = cur.w_inst_bag_ndx;
			const size_t bag_end = next.w_inst_bag_ndx;
			for (size_t bag_ndx = bag_start; bag_ndx < bag_end; bag_ndx++) {
				const BYTE* bag_ptr = pdta.ibag + 8 + bag_ndx * sizeof(spec::SfInstBag);
				spec::SfInstBag bag_cur, bag_next;
				std::memcpy(&bag_cur, bag_ptr, sizeof(spec::SfInstBag));
				std::memcpy(&bag_next, bag_ptr + sizeof(spec::SfInstBag), sizeof(spec::SfInstBag));

				// IMOD is skipped rn...
				const size_t gen_start = bag_cur.w_inst_gen_ndx;
				const size_t gen_end = bag_next.w_inst_gen_ndx;

				const BYTE* gen_ptr = pdta.igen + 8 + gen_start * sizeof(spec::SfInstGenList);
				SfInstrumentZone zone(IZoneHandle(0));
				if (auto err = LoadGenerators(zone, gen_ptr, gen_end - gen_start)) {
					return err;
				}

				if (!zone.IsEmpty()) {
					if (bag_ndx == bag_start && !zone.HasGenerator(SfGenSampleID)) {
						rec.GetGlobalZone().MoveProperties(std::move(zone));
					} else if (zone.HasGenerator(SfGenSampleID)) {
						rec.NewZone().MoveProperties(std::move(zone));
					}
				}
			}
		}
		return SF2ML_SUCCESS;
}

auto SF2ML::loader::LoadSamples(SmplContainer& smpls, const SfbkMap& sfbk) -> SF2ML::SF2MLError {
	SampleBitDepth bit_depth = SampleBitDepth::Signed16;
	const auto& sdta = sfbk.sdta;
	const auto& pdta = sfbk.pdta;

	ChunkHead ck_head;
	std::memcpy(&ck_head, pdta.shdr, sizeof(ck_head));
	const DWORD shdr_size = ck_head.ck_size;
	const BYTE* shdr_data = pdta.shdr + 8;

	const BYTE* smpl_data = nullptr;
	DWORD smpl_size = 0;
	const BYTE* sm24_data = nullptr;
	DWORD sm24_size = 0;

	if (sdta.smpl) {
		bit_depth = SampleBitDepth::Signed16;
		smpl_data = sdta.smpl + 8;
		std::memcpy(&smpl_size, sdta.smpl + 4, sizeof(DWORD));

		if (sdta.sm24) {
			bit_depth = SampleBitDepth::Signed24;
			sm24_data = sdta.sm24 + 8;
			std::memcpy(&sm24_size, sdta.sm24 + 4, sizeof(DWORD));
		}
	}

	std::size_t sample_count = shdr_size / sizeof(spec::SfSample);
	if (sample_count == 0) { // shdr chunk should at least contain 1 terminal record.
		return SF2ML_MISSING_TERMINAL_RECORD;
	}
	sample_count--;

	// read shdr chunk
	for (DWORD id = 0; id < sample_count; id++) {
		spec::SfSample cur_shdr;
		std::memcpy(&cur_shdr, shdr_data + id * sizeof(spec::SfSample), sizeof(spec::SfSample));

		SfSample& rec = smpls.NewItem(bit_depth);
		rec.SetName(std::string(reinterpret_cast<const char*>(cur_shdr.ach_sample_name), 20));
		rec.SetSampleRate(cur_shdr.dw_sample_rate);
		rec.SetLoop(
			cur_shdr.dw_startloop - cur_shdr.dw_start,
			cur_shdr.dw_endloop - cur_shdr.dw_start
		);
		rec.SetRootKey(cur_shdr.by_original_key);
		rec.SetPitchCorrection(cur_shdr.ch_correction);
		rec.SetSampleMode(cur_shdr.sf_sample_type);
		if (!IsMonoSample(cur_shdr.sf_sample_type)) {
			// when loading from file, sample id == sample handle
			rec.SetLink(SmplHandle(cur_shdr.w_sample_link));
		}

		if (IsRamSample(cur_shdr.sf_sample_type)) {
			if (smpl_data && cur_shdr.dw_start < cur_shdr.dw_end && cur_shdr.dw_end <= smpl_size / 2) {
				std::vector<BYTE> wav_data;

				if (bit_depth == SampleBitDepth::Signed16) {
					wav_data.resize((cur_shdr.dw_end - cur_shdr.dw_start) * 2);
					std::memcpy(wav_data.data(), &smpl_data[cur_shdr.dw_start * 2], wav_data.size());
				} else { // SampleBitDepth::Signed24
					wav_data.resize((cur_shdr.dw_end - cur_shdr.dw_start) * 3);
					for (size_t i = cur_shdr.dw_start; i < cur_shdr.dw_end; i++) {
						wav_data[3 * i + 0] = sm24_data[i];
						wav_data[3 * i + 1] = smpl_data[2 * i + 0];
						wav_data[3 * i + 2] = smpl_data[2 * i + 1];
					}
				}

				rec.SetWav(std::move(wav_data));
			}
		}
	}
	return SF2ML_SUCCESS;
}

auto SF2ML::loader::LoadGenerators(SfPresetZone& dst, const BYTE* buf, DWORD count) -> SF2ML::SF2MLError {
	for (size_t gen_ndx = 0; gen_ndx < count; gen_ndx++) {
		const BYTE* gen_ptr = buf + gen_ndx * sizeof(spec::SfGenList);
		spec::SfGenList gen;
		std::memcpy(&gen, gen_ptr, sizeof(spec::SfGenList));

		dst.SetGenerator(gen.sf_gen_oper, InterpretGenerator(gen));
	}
	return SF2ML_SUCCESS;
}

auto SF2ML::loader::LoadGenerators(SfInstrumentZone& dst, const BYTE* buf, DWORD count) -> SF2ML::SF2MLError {
	for (size_t gen_ndx = 0; gen_ndx < count; gen_ndx++) {
		const BYTE* gen_ptr = buf + gen_ndx * sizeof(spec::SfInstGenList);
		spec::SfInstGenList gen;
		std::memcpy(&gen, gen_ptr, sizeof(spec::SfInstGenList));

		dst.SetGenerator(gen.sf_gen_oper, InterpretGenerator(gen));
	}
	return SF2ML_SUCCESS;
}

auto SF2ML::loader::LoadModulators(SfPresetZone& dst, const BYTE* buf, DWORD count) -> SF2ML::SF2MLError {
	return SF2MLError();
}

auto SF2ML::loader::LoadModulators(SfInstrumentZone& dst, const BYTE* buf, DWORD count) -> SF2ML::SF2MLError {
	// scan active modulators(not overridden modulators)
	std::map<std::tuple<SFModulator, SFGenerator, SFModulator>, WORD> active;
	for (size_t mod_ndx = 0; mod_ndx < count; mod_ndx++) {
		const BYTE* mod_ptr = buf + mod_ndx * sizeof(spec::SfInstModList);
		spec::SfInstModList mod;
		std::memcpy(&mod, mod_ptr, sizeof(mod));

		// ignore modulators with link in AmtSrcOper
		if (mod.sf_mod_amt_src_oper == SfModCtrlLink) {
			continue;
		}
		// ignore the preceding modulators if srcoper, destoper, amtsrcoper are the same
		active[std::tuple(mod.sf_mod_src_oper, mod.sf_mod_dest_oper, mod.sf_mod_amt_src_oper)] = mod_ndx;
	}

	// add modulators (except circular links / bad links)
	recursive::InitDFS(active);
	for (const auto& [mod_op, mod_ndx] : active) {
		bool illegal = recursive::DFSModulators(mod_ndx, buf, count);
		if (illegal) {
			continue;
		}
		
	}

	return SF2MLError();
}
