#include "sfserializer.hpp"

SF2ML::DWORD CalculateInfoSize(const SF2ML::SfInfo& infos) {
	using namespace SF2ML;
	auto sizeof_zstr_ck = [](const auto& zstr) -> DWORD {
		return sizeof(ChunkHead) + zstr.length() + 1;
	};
	
	auto sizeof_opt_zstr_ck = []<typename T>(const std::optional<T> zstr_opt) -> DWORD {
		if (zstr_opt) {
			return sizeof(ChunkHead) + zstr_opt->length() + 1;
		} else {
			return 0;
		}
	};

	return sizeof(ChunkHead) + sizeof(FOURCC)
		+ sizeof(ChunkHead) + sizeof(spec::SfVersionTag)
		+ sizeof_zstr_ck(infos.GetSoundEngine())
		+ sizeof_zstr_ck(infos.GetBankName())
		+ sizeof_opt_zstr_ck(infos.GetSoundRomName())
		+ (infos.GetSoundRomVersion() ? sizeof(ChunkHead) + sizeof(spec::SfVersionTag) : 0)
		+ sizeof_opt_zstr_ck(infos.GetCreationDate())
		+ sizeof_opt_zstr_ck(infos.GetAuthor())
		+ sizeof_opt_zstr_ck(infos.GetTargetProduct())
		+ sizeof_opt_zstr_ck(infos.GetCopyrightMessage())
		+ sizeof_opt_zstr_ck(infos.GetComments())
		+ sizeof_opt_zstr_ck(infos.GetToolUsed());
}

SF2ML::DWORD CalculatePresetSize(const SF2ML::PresetContainer& presets) {
	using namespace SF2ML;
	DWORD pdhr_ck_size = sizeof(ChunkHead) + (presets.Count() + 1) * sizeof(spec::SfPresetHeader);
	DWORD pbag_ck_size = sizeof(ChunkHead);
	DWORD pmod_ck_size = sizeof(ChunkHead) + sizeof(spec::SfModList); // no mod support
	DWORD pgen_ck_size = sizeof(ChunkHead);

	DWORD bag_count = 0;
	for (const SfPreset& preset : presets) {
		preset.ForEachZone([&](const SfPresetZone& zone) {
			if (!zone.IsEmpty()) {
				bag_count++;
				pgen_ck_size += zone.RequiredSize();
			}
		});
	}
	
	pbag_ck_size += (bag_count + 1) * sizeof(spec::SfPresetBag);
	pgen_ck_size += sizeof(spec::SfGenList);

	return pdhr_ck_size + pbag_ck_size + pmod_ck_size + pgen_ck_size;
}

SF2ML::DWORD CalculateInstSize(const SF2ML::InstContainer& insts) {
	using namespace SF2ML;
	DWORD inst_ck_size = sizeof(ChunkHead) + (insts.Count() + 1) * sizeof(spec::SfInst);
	DWORD ibag_ck_size = sizeof(ChunkHead);
	DWORD imod_ck_size = sizeof(ChunkHead);
	DWORD igen_ck_size = sizeof(ChunkHead);

	DWORD bag_count = 0;
	for (const SfInstrument& inst : insts) {
		inst.ForEachZone([&](const SfInstrumentZone& zone) {
			if (!zone.IsEmpty()) {
				bag_count++;
				igen_ck_size += zone.GeneratorCount() * sizeof(spec::SfInstGenList);
				imod_ck_size += zone.ModulatorCount() * sizeof(spec::SfInstModList);
			}
		});
	}
	
	ibag_ck_size += (bag_count + 1) * sizeof(spec::SfInstBag);
	igen_ck_size += sizeof(spec::SfInstGenList);
	imod_ck_size += sizeof(spec::SfModList);

	return inst_ck_size + ibag_ck_size + imod_ck_size + igen_ck_size;
}

SF2ML::DWORD CalculateShdrSize(const SF2ML::SmplContainer& smpls, unsigned z_zone) {
	using namespace SF2ML;
	return sizeof(ChunkHead) + (smpls.Count() + 1) * sizeof(spec::SfSample);
}

SF2ML::DWORD CalculateSdtaSize(const SF2ML::SmplContainer& smpls, unsigned z_zone) {
	using namespace SF2ML;
	DWORD sample16_sz = 0;
	DWORD sample24_sz = 0;

	SampleBitDepth bit_depth = GetBitDepth(smpls);

	for (const auto& sample : smpls) {
		if (bit_depth == SampleBitDepth::Signed16) {
			sample16_sz += sample.GetSampleCount() * 2 + z_zone;
		} else {
			sample16_sz += sample.GetSampleCount() * 2 + z_zone;
			sample24_sz += sample.GetSampleCount() + z_zone / 2;
		}
	}

	DWORD smpl_size = sizeof(ChunkHead) + sample16_sz;
	DWORD sm24_size = (bit_depth == SampleBitDepth::Signed24) ? sizeof(ChunkHead) + sample24_sz : 0;
	if (sm24_size % 2 == 1) {
		sm24_size++;
	}

	DWORD sdta_size = sizeof(ChunkHead) + sizeof(FOURCC) + smpl_size + sm24_size;
	return sdta_size;
}

auto SF2ML::serializer::CalculateRiffSize(const SfInfo& infos,
										  const PresetContainer& presets,
										  const InstContainer& insts,
										  const SmplContainer& smpls,
										  unsigned z_zone) -> SF2ML::DWORD {

	DWORD info_size = CalculateInfoSize(infos);
	DWORD sdta_size = CalculateSdtaSize(smpls, z_zone);
	DWORD pdta_size = sizeof(ChunkHead) + sizeof(FOURCC)
					+ CalculatePresetSize(presets)
					+ CalculateInstSize(insts)
					+ CalculateShdrSize(smpls, z_zone);

	return sizeof(ChunkHead) + sizeof(FOURCC) + info_size + sdta_size + pdta_size;
}

auto SF2ML::serializer::SerializeRiff(BYTE* dst,
									  BYTE** end,
									  const SfInfo& infos,
									  const PresetContainer& presets,
									  const InstContainer& insts,
									  const SmplContainer& smpls,
									  unsigned z_zone)
									  -> SF2ML::SF2MLError {

	BYTE* pos = dst;
	
	// serialize RIFF/*
	std::memcpy(pos, "RIFF", 4);
	BYTE* riff_ck_size_ptr = pos + 4;
	std::memcpy(pos + 8, "sfbk", 4);
	pos += 12;
	BYTE* next = pos;

	// serialize RIFF/INFO/*
	if (auto err = SerializeInfos(pos, &next, infos)) {
		return err;
	}
	pos = next;

	// serialize RIFF/sdta/*
	if (auto err = SerializeSDTA(pos, &next, smpls, z_zone)) {
		return err;
	}
	pos = next;

	// serialize RIFF/pdta/*
	std::memcpy(pos, "LIST", 4);
	BYTE* pdta_ck_size_ptr = pos + 4;
	std::memcpy(pos + 8, "pdta", 4);
	pos += 12;

	if (auto err = SerializePresets(pos, &next, presets, insts)) {
		return err;
	}
	pos = next;
	if (auto err = SerializeInstruments(pos, &next, insts, smpls)) {
		return err;
	}
	pos = next;
	if (auto err = SerializeSHDR(pos, &next, smpls, z_zone)) {
		return err;
	}
	pos = next;
	
	DWORD pdta_ck_size = pos - pdta_ck_size_ptr - 4;
	std::memcpy(pdta_ck_size_ptr, &pdta_ck_size, sizeof(DWORD));

	DWORD riff_ck_size = pos - riff_ck_size_ptr - 4;
	std::memcpy(riff_ck_size_ptr, &riff_ck_size, sizeof(DWORD));

	if (end) {
		*end = pos;
	}
	
	return SF2ML_SUCCESS;
}


auto SF2ML::serializer::SerializeInfos(BYTE* dst, BYTE** end, const SfInfo& src) -> SF2ML::SF2MLError {
	BYTE* pos = dst;

	std::memcpy(pos, "LIST", 4);
	BYTE* const size_ptr = pos + 4;
	std::memcpy(pos + 8, "INFO", 4);	
	pos += 12;

	auto serialize_zstr = [&pos](const char* fourcc, const std::string& name) {
		std::memcpy(pos, fourcc, 4);
		pos += 4;
		DWORD len = name.length() + 1; // std::min<std::size_t>(name.length(), 255);
		std::memcpy(pos, &len, sizeof(DWORD));
		pos += 4;
		std::memcpy(pos, name.c_str(), len-1);
		pos += len-1;
		*pos = 0;
		pos++;
	};
	auto serialize_vtag = [&pos](const char* fourcc, const VersionTag& vtag) {
		std::memcpy(pos, fourcc, 4);
		const DWORD sz = sizeof(spec::SfVersionTag);
		std::memcpy(pos + 4, &sz, sizeof(DWORD));
		pos += 8;
		spec::SfVersionTag res {
			.w_major = vtag.major,
			.w_minor = vtag.minor
		};
		std::memcpy(pos, &res, sizeof(res));
		pos += sizeof(res);
	};

	serialize_vtag("ifil", src.GetSoundFontVersion());
	serialize_zstr("isng", src.GetSoundEngine());
	serialize_zstr("INAM", src.GetBankName());

	auto sound_rom_name = src.GetSoundRomName();
	auto sound_rom_version = src.GetSoundRomVersion();
	auto creation_date = src.GetCreationDate();
	auto author = src.GetAuthor();
	auto target_product = src.GetTargetProduct();
	auto copyright_msg = src.GetCopyrightMessage();
	auto comments = src.GetComments();
	auto sf_tools = src.GetToolUsed();

	if (sound_rom_name) {
		serialize_zstr("irom", *sound_rom_name);
	}
	if (sound_rom_version) {
		serialize_vtag("iver", *sound_rom_version);
	}
	if (creation_date) {
		serialize_zstr("ICRD", *creation_date);
	}
	if (author) {
		serialize_zstr("IENG", *author);
	}
	if (target_product) {
		serialize_zstr("IPRD", *target_product);
	}
	if (copyright_msg) {
		serialize_zstr("ICOP", *copyright_msg);
	}
	if (comments) {
		serialize_zstr("ICMT", *comments);
	}
	if (sf_tools) {
		serialize_zstr("ISFT", *sf_tools);
	}

	DWORD size = pos - dst - sizeof(ChunkHead);
	std::memcpy(size_ptr, &size, sizeof(size));

	if (end) {
		*end = pos;
	}
	return SF2ML_SUCCESS;
}

auto SF2ML::serializer::SerializePresets(BYTE* dst, BYTE** end,
										 const PresetContainer& src,
										 const InstContainer& inst_info) -> SF2ML::SF2MLError {
	BYTE* pos = dst;

	// serialize ./phdr
	BYTE* const phdr_head = pos;
	std::memcpy(phdr_head, "phdr", 4);
	DWORD phdr_ck_size = (src.Count() + 1) * sizeof(spec::SfPresetHeader);
	std::memcpy(phdr_head + 4, &phdr_ck_size, sizeof(DWORD));
	pos += 8;
	DWORD bag_idx = 0;
	for (const SfPreset& preset : src) {
		spec::SfPresetHeader bits {};
		std::string name = preset.GetName();
		std::memcpy(bits.ach_preset_name, name.c_str(), std::min<std::size_t>(name.length(), 20));
		bits.w_preset_bag_ndx = bag_idx;
		bits.w_preset = preset.GetPresetNumber();
		bits.w_bank = preset.GetBankNumber();
		std::memcpy(pos, &bits, sizeof(bits));
		bag_idx += preset.CountZones();
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
	WORD gen_idx = 0;
	for (const SfPreset& preset : src) {
		preset.ForEachZone([&pos, &gen_idx](const SfPresetZone& zone) {
			if (!zone.IsEmpty()) {
				spec::SfPresetBag bits;
				bits.w_gen_ndx = gen_idx;
				bits.w_mod_ndx = 0;
				std::memcpy(pos, &bits, sizeof(bits));
				gen_idx += zone.GeneratorCount();
				pos += sizeof(bits);
			}
		});
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
	for (const SfPreset& preset : src) {
		SF2MLError failed = SF2ML_SUCCESS;
		preset.ForEachZone([&](const SfPresetZone& zone) {
			if (!failed && !zone.IsEmpty()) {
				BYTE* next = pos;
				if (auto err = SerializeGenerators(pos, &next, zone, inst_info)) {
					failed = err;
					if (end) {
						*end = next;
					}
					return;
				}
				pos = next;
			}
		});
		if (failed) {
			return failed;
		}
	}
	spec::SfGenList end_of_pgen {};
	std::memcpy(pos, &end_of_pgen, sizeof(end_of_pgen));
	pos += sizeof(end_of_pgen);

	if (end) {
		*end = pos;
	}

	return SF2ML_SUCCESS;
}

auto SF2ML::serializer::SerializeInstruments(BYTE* dst, BYTE** end,
											 const InstContainer& src,
											 const SmplContainer& smpl_info) -> SF2ML::SF2MLError {
	BYTE* pos = dst;

	// serialize ./inst
	BYTE* const inst_head = pos;
	std::memcpy(inst_head, "inst", 4);
	DWORD inst_ck_size = (src.Count() + 1) * sizeof(spec::SfInst);
	std::memcpy(inst_head + 4, &inst_ck_size, sizeof(DWORD));
	pos += 8;
	WORD bag_idx = 0;
	for (const SfInstrument& inst : src) {
		spec::SfInst bits {};
		std::string name = inst.GetName();
		std::memcpy(bits.ach_inst_name, name.c_str(), std::min<std::size_t>(name.length(), 20));
		bits.w_inst_bag_ndx = bag_idx;
		std::memcpy(pos, &bits, sizeof(bits));
		bag_idx += inst.CountZones();
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
	WORD gen_idx = 0;
	WORD mod_idx = 0;
	for (const SfInstrument& inst : src) {
		inst.ForEachZone([&](const SfInstrumentZone& zone) {
			if (!zone.IsEmpty()) {
				spec::SfInstBag bits;
				bits.w_inst_gen_ndx = gen_idx;
				bits.w_inst_mod_ndx = mod_idx;
				std::memcpy(pos, &bits, sizeof(bits));
				gen_idx += zone.GeneratorCount();
				mod_idx += zone.ModulatorCount();
				pos += sizeof(bits);
			}
		});
	}
	spec::SfInstBag end_of_ibag { gen_idx, mod_idx };
	std::memcpy(pos, &end_of_ibag, sizeof(end_of_ibag));
	pos += sizeof(end_of_ibag);

	// serialize ./imod
	BYTE* const imod_head = pos;
	std::memcpy(imod_head, "imod", 4);
	DWORD imod_ck_size = (mod_idx + 1) * sizeof(spec::SfInstModList);
	std::memcpy(imod_head + 4, &imod_ck_size, sizeof(DWORD));
	pos += 8;
	for (const SfInstrument& inst : src) {
		SF2MLError failed = SF2ML_SUCCESS;
		inst.ForEachZone([&](const SfInstrumentZone& zone) {
			BYTE* next = pos;
			if (!failed && !zone.IsEmpty()) {
				if (auto err = SerializeModulators(pos, &next, zone)) {
					failed = err;
					if (end) {
						*end = next;
					}
					return;
				}
				pos = next;
			}
		});
		if (failed) {
			return failed;
		}
	}
	spec::SfInstModList end_of_imod {};
	std::memcpy(pos, &end_of_imod, sizeof(end_of_imod));
	pos += sizeof(end_of_imod);

	// serialize ./igen
	BYTE* const igen_head = pos;
	std::memcpy(igen_head, "igen", 4);
	DWORD igen_ck_size = (gen_idx + 1) * sizeof(spec::SfInstGenList);
	std::memcpy(igen_head + 4, &igen_ck_size, sizeof(DWORD));
	pos += 8;
	for (const SfInstrument& inst : src) {
		SF2MLError failed = SF2ML_SUCCESS;
		inst.ForEachZone([&](const SfInstrumentZone& zone) {
			BYTE* next = pos;
			if (!failed && !zone.IsEmpty()) {
				if (auto err = SerializeGenerators(pos, &next, zone, smpl_info)) {
					failed = err;
					if (end) {
						*end = next;
					}
					return;
				}
				pos = next;
			}
		});
		if (failed) {
			return failed;
		}
	}
	spec::SfInstGenList end_of_igen {};
	std::memcpy(pos, &end_of_igen, sizeof(end_of_igen));
	pos += sizeof(end_of_igen);

	if (end) {
		*end = pos;
	}

	return SF2ML_SUCCESS;
}

auto SF2ML::serializer::SerializeSDTA(BYTE* dst, BYTE** end, const SmplContainer& src, unsigned z_zone) -> SF2ML::SF2MLError {
	BYTE* pos = dst;

	SampleBitDepth bit_depth = GetBitDepth(src);
	
	// serialize ./sdta
	BYTE* const sdta_head = pos;
	std::memcpy(sdta_head, "LIST", 4);
	pos += 8;
	std::memcpy(pos, "sdta", 4);
	pos += 4;
	
	// serialize ./sdta/smpl
	BYTE* const smpl_head = pos;
	std::memcpy(smpl_head, "smpl", 4);
	pos += 8;
	if (bit_depth == SampleBitDepth::Signed16) {
		for (const auto& sample : src) {
			std::memcpy(pos, sample.GetWav().data(), sample.GetWav().size());
			pos += sample.GetWav().size();
			std::memset(pos, 0, z_zone);
			pos += z_zone;
		}
	} else {
		for (const auto& sample : src) {
			DWORD smpl_count = sample.GetWav().size() / 3;
			for (DWORD idx = 0; idx < smpl_count; idx++) {
				pos[0] = sample.GetWav()[3 * idx + 1];
				pos[1] = sample.GetWav()[3 * idx + 2];
				pos += 2;
			}
			std::memset(pos, 0, z_zone);
			pos += z_zone;
		}
	}
	DWORD smpl_sz = pos - smpl_head - 8;
	std::memcpy(smpl_head + 4, &smpl_sz, sizeof(smpl_sz));

	// serialize ./sdta/sm24
	if (bit_depth == SampleBitDepth::Signed24) {
		BYTE* const sm24_head = pos;
		std::memcpy(sm24_head, "sm24", 4);
		pos += 8;
		for (const auto& sample : src) {
			DWORD smpl_count = sample.GetWav().size() / 3;
			for (DWORD idx = 0; idx < smpl_count; idx++) {
				*pos = sample.GetWav()[3 * idx + 0];
				pos++;
			}
			std::memset(pos, 0, z_zone / 2);
			pos += z_zone / 2;
		}
		DWORD sm24_sz = pos - sm24_head - 8;
		if (sm24_sz % 2 == 1) {
			*pos = 0;
			pos++;
			sm24_sz++;
		}
		std::memcpy(sm24_head + 4, &sm24_sz, sizeof(sm24_sz));
	}

	DWORD sdta_sz = pos - sdta_head - 8;
	std::memcpy(sdta_head + 4, &sdta_sz, sizeof(sdta_sz));

	if (end) {
		*end = pos;
	}
	return SF2ML_SUCCESS;
}

auto SF2ML::serializer::SerializeSHDR(BYTE* dst, BYTE** end,
									  const SmplContainer& src,
									  unsigned z_zone) -> SF2ML::SF2MLError {
	BYTE* pos = dst;

	BYTE* const shdr_head = pos;
	std::memcpy(shdr_head, "shdr", 4);
	pos += 8;

	DWORD smpl_idx = 0; // refers to samples in smpl chunk (unit: 16/24 bit sample)
	for (const auto& sample : src) {
		DWORD smpl_count = sample.GetSampleCount();

		spec::SfSample shdr {};
		std::string name = sample.GetName();
		std::memcpy(shdr.ach_sample_name, name.c_str(), std::min<std::size_t>(name.length(), 20));
		shdr.dw_start        = smpl_idx;
		shdr.dw_end          = smpl_idx + smpl_count;
		shdr.dw_startloop    = smpl_idx + sample.GetLoop().first;
		shdr.dw_endloop      = smpl_idx + sample.GetLoop().second;
		shdr.dw_sample_rate  = sample.GetSampleRate();
		shdr.by_original_key = sample.GetRootKey();
		shdr.ch_correction   = sample.GetPitchCorrection();
		shdr.sf_sample_type  = sample.GetSampleMode();
		if (IsMonoSample(shdr.sf_sample_type)) {
			shdr.w_sample_link = 0;
		} else {
			auto smpl_id = src.GetID(*sample.GetLink());
			if (smpl_id) {
				shdr.w_sample_link = smpl_id.value();
			} else {
				return SF2ML_NO_SUCH_SAMPLE;
			}
		}

		smpl_idx += smpl_count + z_zone / 2; // update sample starting point
		std::memcpy(pos, &shdr, sizeof(shdr));
		pos += sizeof(shdr);
	}
	// Append EOS (End Of Sample) Entry
	spec::SfSample eos { "EOS" };
	std::memcpy(pos, &eos, sizeof(eos));
	pos += sizeof(eos);

	DWORD shdr_sz = pos - shdr_head - 8;
	std::memcpy(shdr_head + 4, &shdr_sz, sizeof(shdr_sz));
	
	if (end) {
		*end = pos;
	}
	return SF2ML_SUCCESS;
}

auto SF2ML::serializer::SerializeGenerators(BYTE* dst, BYTE** end,
											const SfPresetZone& src,
											const InstContainer& inst_info) -> SF2ML::SF2MLError {
	// pointer to keep track on where to put generators
	BYTE* pos = dst;
	// helper function for serializing generators (except Instrument generator because that is a special case)
	const auto append_generator = [&pos](SFGenerator type, SfGenAmount amt) {
		spec::SfGenList bits;
		bits.sf_gen_oper = type;
		std::visit([&bits](auto&& arg) {
			using T = std::decay_t<decltype(arg)>;
			if constexpr (std::is_same_v<T, Ranges<BYTE>>) {
				bits.gen_amount.ranges.by_lo = arg.start;
				bits.gen_amount.ranges.by_hi = arg.end;
			} else if constexpr (std::is_same_v<T, WORD>) {
				bits.gen_amount.w_amount = arg;
			} else if constexpr (std::is_same_v<T, SHORT>) {
				bits.gen_amount.sh_amount = arg;
			}
		}, amt);
		std::memcpy(pos, &bits, sizeof(bits));
		pos += sizeof(bits);
	};

	// if it exists, KeyRange generator should be the first entry in GenList
	if (src.HasGenerator(SfGenKeyRange)) {
		append_generator(SfGenKeyRange, src.GetGenerator(SfGenKeyRange));
	}
	// if it exists, VelRange generator should be preceded by KeyRange generator in GenList
	// (though not sure about having VelRange in absence of KeyRange...)
	if (src.HasGenerator(SfGenVelRange)) {
		append_generator(SfGenVelRange, src.GetGenerator(SfGenVelRange));
	}
	// serialize other generators (except Instrument(ID/Handle) generator)
	for (WORD gen_type = 0; gen_type < SfGenEndOper; gen_type++) {
		if (   gen_type == SfGenKeyRange
			|| gen_type == SfGenVelRange
			|| gen_type == SfGenInstrument
			|| !src.HasGenerator(static_cast<SFGenerator>(gen_type))
		) {
			continue;
		}
		append_generator(
			static_cast<SFGenerator>(gen_type),
			src.GetGenerator(static_cast<SFGenerator>(gen_type))
		);
	}
	// if it exists, Instrument generator should be the last entry in GenList
	if (src.HasGenerator(SfGenInstrument)) {
		spec::SfGenList bits;
		bits.sf_gen_oper = SfGenInstrument;
		auto inst_id = inst_info.GetID(*src.GetInstrument());

		// failing condition for serialization: the zone holds an invalidated instrument handle
		// (which means the zone references a deleted instrument that once existed)
		if (inst_id.has_value() == false) {
			if (end) {
				*end = pos;
			}
			return SF2ML_NO_SUCH_INSTRUMENT;
		}
		bits.gen_amount.w_amount = inst_id.value();
		std::memcpy(pos, &bits, sizeof(bits));
		pos += sizeof(bits);
	}

	if (end) {
		*end = pos;
	}

	return SF2ML_SUCCESS;
}

auto SF2ML::serializer::SerializeGenerators(BYTE* dst, BYTE** end,
											const SfInstrumentZone& src,
											const SmplContainer& smpl_info) -> SF2ML::SF2MLError {
	// pointer to keep track on where to put generators
	BYTE* pos = dst;
	// helper function for serializing generators (except SampleID generator because that is a special case)
	const auto append_generator = [&pos](SFGenerator type, SfGenAmount amt) {
		spec::SfInstGenList bits;
		bits.sf_gen_oper = type;
		std::visit([&bits](auto&& arg) {
			using T = std::decay_t<decltype(arg)>;
			if constexpr (std::is_same_v<T, Ranges<BYTE>>) {
				bits.gen_amount.ranges.by_lo = arg.start;
				bits.gen_amount.ranges.by_hi = arg.end;
			} else if constexpr (std::is_same_v<T, WORD>) {
				bits.gen_amount.w_amount = arg;
			} else if constexpr (std::is_same_v<T, SHORT>) {
				bits.gen_amount.sh_amount = arg;
			}
		}, amt);
		std::memcpy(pos, &bits, sizeof(bits));
		pos += sizeof(bits);
	};

	// if it exists, KeyRange generator should be the first entry in GenList
	if (src.HasGenerator(SfGenKeyRange)) {
		append_generator(SfGenKeyRange, src.GetGenerator(SfGenKeyRange));
	}
	// if it exists, VelRange generator should be preceded by KeyRange generator in GenList
	// (though not sure about having VelRange in absence of KeyRange...)
	if (src.HasGenerator(SfGenVelRange)) {
		append_generator(SfGenVelRange, src.GetGenerator(SfGenVelRange));
	}
	// serialize other generators (except Instrument(ID/Handle) generator)
	for (WORD gen_type = 0; gen_type < SfGenEndOper; gen_type++) {
		if (   gen_type == SfGenKeyRange
			|| gen_type == SfGenVelRange
			|| gen_type == SfGenSampleID
			|| !src.HasGenerator(static_cast<SFGenerator>(gen_type))
		) {
			continue;
		}
		append_generator(
			static_cast<SFGenerator>(gen_type),
			src.GetGenerator(static_cast<SFGenerator>(gen_type))
		);
	}
	if (src.HasGenerator(SfGenSampleID)) {
		spec::SfInstGenList bits;
		bits.sf_gen_oper = SfGenSampleID;
		auto sample_id = smpl_info.GetID(*src.GetSampleHandle());
		
		// failing condition for serialization: the zone holds an invalidated instrument handle
		// (which means the zone references a deleted instrument that once existed)
		if (sample_id.has_value() == false) {
			if (end) {
				*end = pos;
			}
			return SF2ML_NO_SUCH_SAMPLE;
		}
		bits.gen_amount.w_amount = sample_id.value();
		std::memcpy(pos, &bits, sizeof(bits));
		pos += sizeof(bits);
	}

	if (end) {
		*end = pos;
	}

	return SF2ML_SUCCESS;
}

template<class... Ts>
struct overloaded : Ts... { using Ts::operator()...; };

#if __cplusplus < 202002L
template<class... Ts>
overloaded(Ts...) -> overloaded<Ts...>;
#endif

namespace {
	SF2ML::SFModulator CalcModSrcBits(SF2ML::GeneralController cc,
									  bool polarity,
									  bool direction,
									  SF2ML::SfModSourceType shape) {

		return static_cast<SF2ML::SFModulator>(cc)
			| (0u << 7u)
			| (direction << 8u)
			| (polarity << 9u)
			| (static_cast<SF2ML::SFModulator>(shape) << 10u);
	}

	SF2ML::SFModulator CalcModSrcBits(SF2ML::MidiController cc,
									  bool polarity,
									  bool direction,
									  SF2ML::SfModSourceType shape) {

		return static_cast<SF2ML::SFModulator>(cc)
			| (1u << 7u)
			| (direction << 8u)
			| (polarity << 9u)
			| (static_cast<SF2ML::SFModulator>(shape) << 10u);
	}
}

auto SF2ML::serializer::SerializeModulators(BYTE* dst, BYTE** end,
											const SfPresetZone& src) -> SF2ML::SF2MLError {
	return SF2ML_SUCCESS;
}

auto SF2ML::serializer::SerializeModulators(BYTE* dst, BYTE** end,
											const SfInstrumentZone& src) -> SF2ML::SF2MLError {

	src.ForEachModulators([&](const SfModulator& mod) {
		bool p1 = mod.GetSourcePolarity();
		bool d1 = mod.GetSourceDirection();
		auto s1 = mod.GetSourceShape();
		bool p2 = mod.GetAmtSourcePolarity();
		bool d2 = mod.GetAmtSourceDirection();
		auto s2 = mod.GetAmtSourceShape();

		spec::SfInstModList bits;
		bits.mod_amount = mod.GetModAmount();
		bits.sf_mod_trans_oper = mod.GetTransform();
		bits.sf_mod_src_oper = std::visit(
			[&](auto&& cc) { return CalcModSrcBits(cc, p1, d1, s1); },
			mod.GetSourceController()
		);
		bits.sf_mod_amt_src_oper = std::visit(
			[&](auto&& cc) { return CalcModSrcBits(cc, p2, d2, s2); },
			mod.GetSourceController()
		);

		// TODO: handle when GetModID returns std::nullopt;
		bits.sf_mod_dest_oper = std::visit(overloaded {
			[&](SFGenerator x) { return x; },
			[&](ModHandle   x) { return static_cast<SFGenerator>((1 << 15) | *src.GetModID(x)); }
		}, mod.GetDestination());

		std::memcpy(dst, &bits, sizeof(bits));
		dst += sizeof(bits);
	});

	if (end) {
		*end = dst;
	}

	return SF2ML_SUCCESS;
}
