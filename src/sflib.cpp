#include "sfmap.hpp"
#include "sample_manager.hpp"
#include "instrument_manager.hpp"
#include "preset_manager.hpp"

#include <sfml/sfinstrument.hpp>
#include <sfml/sfpreset.hpp>
#include <sfml/sfsample.hpp>
#include <sfml/sfinfo.hpp>
#include <sfml/sfhandle.hpp>
#include <sfml/sfspec.hpp>
#include <sflib.hpp>

#include <cstddef>
#include <memory>
#include <cstring>
#include <iostream>

namespace sflib {
	static std::size_t GetFileSize(std::ifstream& ifs) {
		ifs.seekg(0, std::ios::end);
		std::size_t sz = ifs.tellg();
		ifs.seekg(0, std::ios::beg);
		sz -= ifs.tellg();
		return sz;
	}

	class SoundFontImpl {
		friend class SoundFont;

		public:
		SoundFontImpl() : bit_depth(SampleBitDepth::Signed16),
		                  sample_manager(samples, bit_depth),
		                  instrument_manager(instruments, sample_manager),
						  preset_manager(presets, instrument_manager) {}
		private:

		SflibError LoadInfo(const SfbkMap& sfbk);
		SflibError LoadSamples(const SfbkMap& sfbk);
		SflibError LoadInstruments(const SfbkMap& sfbk);
		SflibError LoadPresets(const SfbkMap& sfbk);

		SflibError LoadPresetZone(SfPresetZone& dst, const BYTE* buf, DWORD count);
		SflibError LoadInstrumentZone(SfInstrumentZone& dst, const BYTE* buf, DWORD count);

		SampleBitDepth bit_depth;
		SfHandleInterface<SfSample, SmplHandle> samples;
		SfHandleInterface<SfInstrument, InstHandle> instruments;
		SfHandleInterface<SfPreset, PresetHandle> presets;

		SfInfo infos;
		SampleManager sample_manager;
		InstrumentManager instrument_manager;
		PresetManager preset_manager;
	};

	SoundFont::SoundFont() {
		pimpl = std::make_unique<SoundFontImpl>();
	}
	SoundFont::~SoundFont() {}

	SflibError SoundFont::Load(std::ifstream &ifs)
	{
		// reset state
		pimpl = std::make_unique<SoundFontImpl>();

		std::size_t sz = GetFileSize(ifs);

		if (sz < 8) {
			return SFLIB_FAILED;
		}

		std::vector<BYTE> riff_content(sz);
		ifs.read(reinterpret_cast<char*>(riff_content.data()), sz);
		ChunkHead riff_head;
		std::memcpy(&riff_head, &riff_content[0], sizeof(riff_head));
		
		if (!CheckFOURCC(riff_head.ck_id, "RIFF") || riff_head.ck_size + 8 > sz) {
			return SFLIB_FAILED;
		}

		SfbkMap sfbk_map;
		if (auto err = GetSfbkMap(sfbk_map, &riff_content[8], riff_head.ck_size)) {
			return err;
		}

		if (auto err = pimpl->LoadInfo(sfbk_map)) {
			return err;
		}
		if (auto err = pimpl->LoadSamples(sfbk_map)) {
			return err;
		}
		if (auto err = pimpl->LoadInstruments(sfbk_map)) {
			return err;
		}
		if (auto err = pimpl->LoadPresets(sfbk_map)) {
			return err;
		}

		return SFLIB_SUCCESS;
	}

	SflibError SoundFont::Save(std::ofstream& ofs) {
		DWORD info_size = pimpl->infos.InfoSize();
		DWORD sdta_size = pimpl->sample_manager.SdtaSize();
		DWORD pdta_size = sizeof(ChunkHead) + sizeof(FOURCC)
			+ pimpl->preset_manager.ChunkSize()
			+ pimpl->instrument_manager.ChunkSize()
			+ pimpl->sample_manager.ShdrSize();

		DWORD riff_size = sizeof(ChunkHead) + sizeof(FOURCC)
						+ info_size + sdta_size + pdta_size;

		DWORD riff_data_size = riff_size - sizeof(ChunkHead);

		std::vector<BYTE> bytes(riff_size);
		std::memcpy(&bytes[0], "RIFF", 4);
		std::memcpy(&bytes[4], &riff_data_size, sizeof(riff_data_size));
		std::memcpy(&bytes[8], "sfbk", 4);

		BYTE* pos = &bytes[0] + 12;
		BYTE* end = pos;

		if (auto err = pimpl->infos.Serialize(pos, &end)) {
			return err;
		}
		pos = end;
		if (auto err = pimpl->sample_manager.SerializeSDTA(pos, &end)) {
			return err;
		}
		pos = end;

		std::memcpy(pos, "LIST", 4);
		DWORD pdta_data_size = pdta_size - sizeof(ChunkHead);
		std::memcpy(pos + 4, &pdta_data_size, 4);
		std::memcpy(pos + 8, "pdta", 4);
		pos += 12;

		if (auto err = pimpl->preset_manager.Serialize(pos, &end)) {
			return err;
		}
		pos = end;
		if (auto err = pimpl->instrument_manager.Serialize(pos, &end)) {
			return err;
		}
		pos = end;
		
		if (auto err = pimpl->sample_manager.SerializeSHDR(pos, &end)) {
			return err;
		}
		pos = end;

		ofs.write(reinterpret_cast<char*>(bytes.data()), bytes.size());		
		return SFLIB_SUCCESS;
	}

	SflibError SoundFont::ExportWav(std::ofstream& ofs, SmplHandle sample) {
		return SFLIB_FAILED;
	}

	SfInfo& SoundFont::GetInfo() {
		return pimpl->infos;
	}

	void SoundFont::SetInfo(const SfInfo& info) {
		pimpl->infos = info;
	}

	SflibError SoundFontImpl::LoadInfo(const SfbkMap& sfbk) {
		const auto& info = sfbk.info;

		ChunkHead ck_head;
		auto validate_zstr = [&ck_head](const BYTE* ck_data) -> bool {
			return ck_data[ck_head.ck_size - 1] == 0x00;
		};

		// ./ifil
		std::memcpy(&ck_head, info.ifil, sizeof(ck_head));
		if (ck_head.ck_size != 4) {
			return SFLIB_INVALID_CK_SIZE;
		}
		spec::SfVersionTag ver;
		std::memcpy(&ver, info.ifil + 8, sizeof(ver));
		infos.sf_version.major = ver.w_major;
		infos.sf_version.minor = ver.w_minor;

		// ./isng
		std::memcpy(&ck_head, info.isng, sizeof(ck_head));
		if (validate_zstr(info.isng + 8)) {
			infos.target_sound_engine.assign(reinterpret_cast<const char*>(info.isng + 8));
		} else {
			infos.target_sound_engine.assign("EMU8000");
		}

		// ./inam
		std::memcpy(&ck_head, info.inam, sizeof(ck_head));
		if (validate_zstr(info.inam + 8)) {
			infos.sf_bank_name.assign(reinterpret_cast<const char*>(info.inam + 8));
		} else {
			return SFLIB_ZSTR_CHECK_FAILED;
		}

		// set optional zero terminated string fields
		auto set_optional_zstr_field = [this](
			std::optional<std::string>& dst,
			const BYTE* ck_head_ptr
		) {
			if (ck_head_ptr) {
				ChunkHead head;
				std::memcpy(&head, ck_head_ptr, sizeof(ck_head));
				if (ck_head_ptr[7 + head.ck_size] == 0x00) {
					dst = std::string(reinterpret_cast<const char*>(ck_head_ptr + 8));
				}
			}	
		};

		set_optional_zstr_field(infos.sound_rom_name, info.irom);
		if (info.iver) {
			std::memcpy(&ck_head, info.iver, sizeof(ck_head));
			if (ck_head.ck_size == 4) {
				spec::SfVersionTag ver;
				std::memcpy(&ver, info.iver + 8, sizeof(ver));
				infos.sound_rom_version = VersionTag {
					.major = ver.w_major,
					.minor = ver.w_minor
				};
			}
		}
		set_optional_zstr_field(infos.creation_date, info.icrd);
		set_optional_zstr_field(infos.author, info.ieng);
		set_optional_zstr_field(infos.target_product, info.iprd);
		set_optional_zstr_field(infos.copyright_msg, info.icop);
		set_optional_zstr_field(infos.comments, info.icmt);
		set_optional_zstr_field(infos.sf_tools, info.isft);
		return SFLIB_SUCCESS;
	}

	SflibError SoundFontImpl::LoadSamples(const SfbkMap& sfbk) {
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
			return SFLIB_MISSING_TERMINAL_RECORD;
		}
		sample_count--;

		// read shdr chunk
		for (DWORD id = 0; id < sample_count; id++) {
			spec::SfSample cur_shdr;
			std::memcpy(&cur_shdr, shdr_data + id * sizeof(spec::SfSample), sizeof(spec::SfSample));

			SfSample& rec = samples.NewItem();
			std::memcpy(rec.sample_name, cur_shdr.ach_sample_name, 20);
			rec.sample_name[20] = 0;

			rec.sample_rate = cur_shdr.dw_sample_rate;
			rec.start_loop  = cur_shdr.dw_startloop - cur_shdr.dw_start;
			rec.end_loop    = cur_shdr.dw_endloop - cur_shdr.dw_start;
			rec.root_key    = cur_shdr.by_original_key;
			rec.pitch_correction = cur_shdr.ch_correction;
			rec.linked_sample = SmplHandle(0);
			rec.sample_type = cur_shdr.sf_sample_type;
			if (rec.sample_type & (leftSample | rightSample)) {
				// when loading from file, sample id == sample handle
				rec.linked_sample = SmplHandle(cur_shdr.w_sample_link);
			}

			// first case: the record is ROM sample(not implemented).
			// second case: the record is RAM sample and has valid sample start, end position.
			// third(?) case: the record is RAM sample, but has invalid sample start, end position.
			// in either case, the record must be loaded in order to maintain their original SampleID.

			if (rec.sample_type & (RomLeftSample & RomRightSample)) {
				return SFLIB_UNIMPLEMENTED;	
			} else if (smpl_data && cur_shdr.dw_start < cur_shdr.dw_end && cur_shdr.dw_end <= smpl_size / 2) {
				if (bit_depth == SampleBitDepth::Signed16) {
					rec.wav_data.resize((cur_shdr.dw_end - cur_shdr.dw_start) * 2);
					std::memcpy(rec.wav_data.data(), &smpl_data[cur_shdr.dw_start * 2], rec.wav_data.size());
				} else { // SampleBitDepth::Signed24
					rec.wav_data.resize((cur_shdr.dw_end - cur_shdr.dw_start) * 3);
					for (size_t i = cur_shdr.dw_start; i < cur_shdr.dw_end; i++) {
						rec.wav_data[3 * i + 0] = sm24_data[i];
						rec.wav_data[3 * i + 1] = smpl_data[2 * i + 0];
						rec.wav_data[3 * i + 2] = smpl_data[2 * i + 1];
					}
				}
			}
		}
		return SFLIB_SUCCESS;
	}
	
	SflibError SoundFontImpl::LoadInstruments(const SfbkMap& sfbk) {
		const auto& pdta = sfbk.pdta;

		DWORD inst_ck_size;
		std::memcpy(&inst_ck_size, pdta.inst + 4, sizeof(DWORD));

		size_t inst_count = inst_ck_size / sizeof(spec::SfInst) - 1;
		for (DWORD id = 0; id < inst_count; id++) {
			const BYTE* cur_ptr = pdta.inst + 8 + id * sizeof(spec::SfInst);
			spec::SfInst cur, next;
			std::memcpy(&cur, cur_ptr, sizeof(spec::SfInst));
			std::memcpy(&next, cur_ptr + sizeof(spec::SfInst), sizeof(spec::SfInst));

			SfInstrument& rec = instruments.NewItem();
			memcpy(rec.inst_name, cur.ach_inst_name, 20);
			rec.inst_name[20] = 0;

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
				LoadInstrumentZone(zone, gen_ptr, gen_end - gen_start);

				if (zone.IsEmpty()) {
					continue;
				}

				if (bag_ndx == bag_start && !zone.HasGenerator(SfGenSampleID)) {
					rec.GetGlobalZone().MoveProperties(std::move(zone));
				} else if (zone.HasGenerator(SfGenSampleID)) {
					rec.NewZone().MoveProperties(std::move(zone));
				}
			}
		}
		return SFLIB_SUCCESS;
	}

	SflibError SoundFontImpl::LoadPresets(const SfbkMap& sfbk) {
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
			std::memcpy(rec.preset_name, cur.ach_preset_name, 20);
			rec.preset_name[20] = 0;
			rec.preset_number = cur.w_preset;
			rec.bank_number = cur.w_bank;

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
				LoadPresetZone(zone, gen_ptr, gen_end - gen_start);

				if (zone.IsEmpty()) {
					continue;
				}

				if (bag_ndx == bag_start && !zone.HasGenerator(SfGenInstrument)) {
					rec.GetGlobalZone().MoveProperties(std::move(zone));
				} else if (zone.HasGenerator(SfGenInstrument)) {
					rec.NewZone().MoveProperties(std::move(zone));
				}
			}
		}

		return SFLIB_SUCCESS;
	}

	auto SoundFont::AddMonoSample(std::ifstream &ifs, std::string_view name, SampleChannel ch) -> SflibResult<SmplHandle> {
		std::size_t size = GetFileSize(ifs);
		std::vector<char> buf(size);
		ifs.read(buf.data(), size);
	
		return AddMonoSample(buf.data(), size, name, ch);
	}

	auto SoundFont::AddMonoSample(const void* file_buf, std::size_t file_size, std::string_view name, SampleChannel ch) -> SflibResult<SmplHandle> {
		return pimpl->sample_manager.AddMono(
			file_buf,
			file_size,
			std::string(name),
			std::nullopt,
			std::nullopt,
			std::nullopt,
			ch
		);
	}

	auto SoundFont::AddStereoSample(std::ifstream& ifs, std::string_view left, std::string_view right) -> SflibResult<std::pair<SmplHandle, SmplHandle>> {
		std::size_t size = GetFileSize(ifs);
		std::vector<char> buf(size);
		ifs.read(buf.data(), size);

		return AddStereoSample(buf.data(), size, left, right);
	}

	auto SoundFont::AddStereoSample(const void* file_buf, std::size_t file_size, std::string_view left, std::string_view right) -> SflibResult<std::pair<SmplHandle, SmplHandle>> {
		return pimpl->sample_manager.AddStereo(
			file_buf,
			file_size,
			std::string(left),
			std::string(right)
		);
	}
	
	auto SoundFont::LinkSamples(SmplHandle left, SmplHandle right) -> SflibError {
		return pimpl->sample_manager.LinkStereo(left, right);
	}

	auto SoundFont::GetSample(SmplHandle smpl) -> SfSample& {
		return *pimpl->samples.Get(smpl);
	}

	void SoundFont::RemoveSample(SmplHandle smpl, RemovalMode rm_mode) {
		pimpl->sample_manager.Remove(smpl, rm_mode);
	}

	auto SoundFont::FindSample(std::function<bool(const SfSample &)> pred) -> std::optional<SmplHandle> {
		for (const auto& sample : pimpl->samples) {
			if (pred(sample)) {
				return sample.GetHandle();
			}
		}
		return std::nullopt;
	}

	auto SoundFont::FindSamples(std::function<bool(const SfSample &)> pred) -> std::vector<SmplHandle> {
		std::vector<SmplHandle> handles;
		for (const auto& sample : pimpl->samples) {
			if (pred(sample)) {
				handles.push_back(sample.GetHandle());
			}
		}
		return handles;
	}

	auto SoundFont::AllSamples() -> std::vector<SmplHandle> {
		auto [first, last] = pimpl->samples.GetAllHandles();
		return std::vector<SmplHandle>(first, last);
	}

	SfInstrument& SoundFont::NewInstrument(std::string_view name) {
		return pimpl->instrument_manager.NewInstrument(std::string(name));
	}

	auto SoundFont::GetInstrument(InstHandle inst) -> SfInstrument& {
		return *pimpl->instruments.Get(inst);
	}

	void SoundFont::RemoveInstrument(InstHandle inst) {
		pimpl->instrument_manager.Remove(inst);
	}

	auto SoundFont::FindInstrument(std::function<bool(const SfInstrument &)> pred) -> std::optional<InstHandle> {
		for (const auto& inst : pimpl->instruments) {
			if (pred(inst)) {
				return inst.GetHandle();
			}
		}
		return std::nullopt;
	}

	auto SoundFont::FindInstruments(std::function<bool(const SfInstrument &)> pred) -> std::vector<InstHandle> {
		std::vector<InstHandle> handles;
		for (const auto& inst : pimpl->instruments) {
			if (pred(inst)) {
				handles.push_back(inst.GetHandle());
			}
		}
		return handles;
	}

	auto SoundFont::AllInstruments() -> std::vector<InstHandle> {
		auto [first, last] = pimpl->instruments.GetAllHandles();
		return std::vector<InstHandle>(first, last);
	}

	SfPreset& SoundFont::NewPreset(std::uint16_t preset_number,
	                               std::uint16_t bank_number,
		                           std::string_view name) {
		return pimpl->preset_manager.NewPreset(
			preset_number, bank_number, std::string(name));
	}

	auto SoundFont::GetPreset(PresetHandle preset) -> SfPreset& {
		return *pimpl->presets.Get(preset);
	}

	void SoundFont::RemovePreset(PresetHandle preset) {
		pimpl->preset_manager.Remove(preset);
	}

	auto SoundFont::FindPreset(std::function<bool(const SfPreset &)> pred) -> std::optional<PresetHandle> {
		for (const auto& preset : pimpl->presets) {
			if (pred(preset)) {
				return preset.GetHandle();
			}
		}
		return std::nullopt;
	}

	auto SoundFont::FindPresets(std::function<bool(const SfPreset &)> pred) -> std::vector<PresetHandle> {
		std::vector<PresetHandle> handles;
		for (const auto& preset : pimpl->presets) {
			if (pred(preset)) {
				handles.push_back(preset.GetHandle());
			}
		}
		return handles;
	}

	auto SoundFont::AllPresets() -> std::vector<PresetHandle> {
		auto [first, last] = pimpl->presets.GetAllHandles();
		return std::vector<PresetHandle>(first, last);
	}

	SflibError SoundFontImpl::LoadPresetZone(SfPresetZone& dst, const BYTE* buf, DWORD count) {
		for (size_t gen_ndx = 0; gen_ndx < count; gen_ndx++) {
			const BYTE* gen_ptr = buf + gen_ndx * sizeof(spec::SfGenList);
			spec::SfGenList gen;
			std::memcpy(&gen , gen_ptr, sizeof(spec::SfGenList));

			dst.generators[gen.sf_gen_oper] = gen.gen_amount;
			dst.active_gens.set(gen.sf_gen_oper);
		}
		if (dst.active_gens[SfGenInstrument]) {
			dst.instrument = InstHandle{dst.generators[SfGenInstrument].w_amount};
		}
		return SFLIB_SUCCESS;
	}

	SflibError SoundFontImpl::LoadInstrumentZone(SfInstrumentZone& dst, const BYTE* buf, DWORD count) {
		for (size_t gen_ndx = 0; gen_ndx < count; gen_ndx++) {
			const BYTE* gen_ptr = buf + gen_ndx * sizeof(spec::SfInstGenList);
			spec::SfInstGenList gen;
			std::memcpy(&gen , gen_ptr, sizeof(spec::SfInstGenList));

			dst.generators[gen.sf_gen_oper] = gen.gen_amount;
			dst.active_gens.set(gen.sf_gen_oper);
		}
		if (dst.active_gens[SfGenSampleID]) {
			dst.sample = SmplHandle{dst.generators[SfGenSampleID].w_amount};
		}
		return SFLIB_SUCCESS;
	}
}