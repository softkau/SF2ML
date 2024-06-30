#include "sfmap.hpp"
#include "sfloader.hpp"
#include "sfserializer.hpp"

#include <sfinstrument.hpp>
#include <sfpreset.hpp>
#include <sfsample.hpp>
#include <sfinfo.hpp>
#include <sfhandle.hpp>
#include <sfspec.hpp>
#include <sf2ml.hpp>

#include <cstddef>
#include <memory>
#include <cstring>
#include <iostream>

namespace SF2ML {
	static std::size_t GetFileSize(std::ifstream& ifs) {
		ifs.seekg(0, std::ios::end);
		std::size_t sz = ifs.tellg();
		ifs.seekg(0, std::ios::beg);
		sz -= ifs.tellg();
		return sz;
	}

	class SoundFontImpl {
		friend class SoundFont;

		auto AddMono(const void* wav_data,
					 std::size_t wav_size,
					 std::string_view name,
					 std::optional<Ranges<DWORD>> loop,
					 std::optional<BYTE> root_key,
					 std::optional<CHAR> pitch_correction,
					 SampleChannel sample_type)
					 -> SF2MLResult<SmplHandle>;
		
		auto AddStereo(const void* wav_data,
					   size_t wav_size,
					   std::string_view name_left,
					   std::string_view name_right,
					   std::optional<Ranges<DWORD>> loop,
					   std::optional<BYTE> root_key,
					   std::optional<CHAR> pitch_correction)
					   -> SF2MLResult<std::pair<SmplHandle, SmplHandle>>;

		auto LinkStereo(SmplHandle left, SmplHandle right) -> SF2MLError;
		void Remove(SmplHandle target, RemovalMode rm_mode);

		SfHandleInterface<SfSample, SmplHandle> samples;
		SfHandleInterface<SfInstrument, InstHandle> instruments;
		SfHandleInterface<SfPreset, PresetHandle> presets;
		SfInfo infos;
	};

	SoundFont::SoundFont() {
		pimpl = std::make_unique<SoundFontImpl>();
	}
	SoundFont::~SoundFont() {}

	SF2MLError SoundFont::Load(std::ifstream& ifs)
	{
		// reset state
		pimpl = std::make_unique<SoundFontImpl>();

		std::size_t sz = GetFileSize(ifs);

		if (sz < 8) {
			return SF2ML_FAILED;
		}

		std::vector<BYTE> riff_content(sz);
		ifs.read(reinterpret_cast<char*>(riff_content.data()), sz);
		ChunkHead riff_head;
		std::memcpy(&riff_head, &riff_content[0], sizeof(riff_head));
		
		if (!CheckFOURCC(riff_head.ck_id, "RIFF") || riff_head.ck_size + 8 > sz) {
			return SF2ML_FAILED;
		}

		SfbkMap sfbk_map;
		if (auto err = GetSfbkMap(sfbk_map, &riff_content[8], riff_head.ck_size)) {
			return err;
		}
		if (auto err = loader::LoadSfbk(pimpl->infos,
										pimpl->presets,
										pimpl->instruments,
										pimpl->samples,
										sfbk_map)) {
			return err;
		}

		return SF2ML_SUCCESS;
	}

	SF2MLError SoundFont::Save(std::ofstream& ofs) {
		DWORD riff_size = serializer::CalculateRiffSize(pimpl->infos,
														pimpl->presets,
														pimpl->instruments,
														pimpl->samples, 46);

		std::vector<BYTE> bytes(riff_size);

		BYTE* end = nullptr;
		if (auto err = serializer::SerializeRiff(bytes.data(),
												 &end,
												 pimpl->infos,
												 pimpl->presets,
												 pimpl->instruments,
												 pimpl->samples, 46)) {
			return err;
		}

		if (riff_size != end - bytes.data()) {
			return SF2ML_FAILED;
		}

		ofs.write(reinterpret_cast<char*>(bytes.data()), bytes.size());		
		return SF2ML_SUCCESS;
	}

	SF2MLError SoundFont::ExportWav(std::ofstream& ofs, SmplHandle sample) {
		return SF2ML_UNIMPLEMENTED;
	}

	SfInfo& SoundFont::Info() {
		return pimpl->infos;
	}

	auto SoundFont::AddMonoSample(std::ifstream &ifs,
								  std::string_view name,
								  std::optional<Ranges<DWORD>> loop,
								  std::optional<BYTE> root_key,
								  std::optional<CHAR> pitch_correction,
								  SampleChannel ch)
								  -> SF2MLResult<SmplHandle> {
		std::size_t size = GetFileSize(ifs);
		std::vector<char> buf(size);
		ifs.read(buf.data(), size);
	
		return AddMonoSample(
			buf.data(), size,
			name,
			loop,
			root_key,
			pitch_correction,
			ch
		);
	}

	auto SoundFont::AddMonoSample(const void* file_buf,
								  std::size_t file_size,
								  std::string_view name,
								  std::optional<Ranges<DWORD>> loop,
								  std::optional<BYTE> root_key,
								  std::optional<CHAR> pitch_correction,
								  SampleChannel ch)
								  -> SF2MLResult<SmplHandle> {
		return pimpl->AddMono(
			file_buf,
			file_size,
			name,
			loop,
			root_key,
			pitch_correction,
			ch
		);
	}

	auto SoundFont::AddStereoSample(std::ifstream& ifs,
									std::string_view left,
									std::string_view right,
									std::optional<Ranges<DWORD>> loop,
									std::optional<BYTE> root_key,
									std::optional<CHAR> pitch_correction)
									-> SF2MLResult<std::pair<SmplHandle, SmplHandle>> {
		std::size_t size = GetFileSize(ifs);
		std::vector<char> buf(size);
		ifs.read(buf.data(), size);

		return AddStereoSample(buf.data(), size, left, right, loop, root_key, pitch_correction);
	}

	auto SoundFont::AddStereoSample(const void* file_buf,
									std::size_t file_size,
									std::string_view left,
									std::string_view right,
									std::optional<Ranges<DWORD>> loop,
									std::optional<BYTE> root_key,
									std::optional<CHAR> pitch_correction)
									-> SF2MLResult<std::pair<SmplHandle, SmplHandle>> {
		return pimpl->AddStereo(
			file_buf,
			file_size,
			left,
			right,
			loop,
			root_key,
			pitch_correction
		);
	}
	
	auto SoundFont::LinkSamples(SmplHandle left, SmplHandle right) -> SF2MLError {
		return pimpl->LinkStereo(left, right);
	}

	auto SoundFont::GetSample(SmplHandle smpl) -> SfSample& {
		return *pimpl->samples.Get(smpl);
	}

	void SoundFont::RemoveSample(SmplHandle smpl, RemovalMode rm_mode) {
		pimpl->Remove(smpl, rm_mode);
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
		return pimpl->instruments.NewItem().SetName(name);
	}

	auto SoundFont::GetInstrument(InstHandle inst) -> SfInstrument& {
		return *pimpl->instruments.Get(inst);
	}

	void SoundFont::RemoveInstrument(InstHandle inst) {
		pimpl->instruments.Remove(inst);
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
		return pimpl->presets.NewItem()
			.SetPresetNumber(preset_number)
			.SetBankNumber(bank_number)
			.SetName(name);
	}

	auto SoundFont::GetPreset(PresetHandle preset) -> SfPreset& {
		return *pimpl->presets.Get(preset);
	}

	void SoundFont::RemovePreset(PresetHandle preset) {
		pimpl->presets.Remove(preset);
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

	auto SoundFontImpl::AddMono(const void* wav_data,
								std::size_t wav_size,
								std::string_view name,
								std::optional<Ranges<DWORD>> loop,
								std::optional<BYTE> root_key,
								std::optional<CHAR> pitch_correction,
								SampleChannel sample_type)
								-> SF2MLResult<SmplHandle> {
		auto [wav_info, err] = wav::ValidateWav(wav_data, wav_size);
		if (err) {
			return { SmplHandle{0}, err };
		}

		SampleBitDepth bit_depth = GetBitDepth(this->samples);
		if (this->samples.Count() == 0) {
			bit_depth = wav_info.bit_depth;
		}

		if (bit_depth != wav_info.bit_depth) {
			return { SmplHandle{0}, SF2ML_INCOMPATIBLE_BIT_DEPTH };
		}
		if (sample_type == SampleChannel::Mono) {
			if (wav_info.num_of_channels != wav::ChannelMono) { // sample_type(arg) is "Mono", but "Stereo" wav data is given
				return { SmplHandle{0}, SF2ML_NOT_MONO_CHANNEL };
			}
		} else {
			if (wav_info.num_of_channels == wav::ChannelMono) { // sample_type(arg) is "Stereo", but "Mono" wav data is given
				return { SmplHandle{0}, SF2ML_NOT_STEREO_CHANNEL };
			}
		}

		const size_t bytes_per_sample = wav_info.bit_depth == SampleBitDepth::Signed16 ? 2 : 3;

		SfSample& rec = samples.NewItem(bit_depth).SetName(name);

		if (loop) {
			rec.SetLoop(loop->start, loop->end);
		} else {
			rec.SetLoop(0, wav_info.wav_size / bytes_per_sample);
		}
		if (root_key) {
			rec.SetRootKey(*root_key);
		}
		if (pitch_correction) {
			rec.SetPitchCorrection(*pitch_correction);
		}
		rec.SetSampleRate(wav_info.sample_rate);

		if (sample_type == SampleChannel::Mono) {
			rec.SetWav(std::vector<BYTE>(wav_info.wav_data, wav_info.wav_data + wav_info.wav_size));
		} else {
			const size_t buf_size = wav_info.wav_size / 2;
			std::vector<BYTE> buf(buf_size);
			const size_t offset = (sample_type == SampleChannel::Left) ? 0 : bytes_per_sample;
			for (size_t buf_idx = 0, blk_idx = 0; buf_idx < buf_size; buf_idx += bytes_per_sample, blk_idx++) {
				std::memcpy(
					&buf[buf_idx],
					&wav_info.wav_data[blk_idx * wav_info.block_align + offset],
					bytes_per_sample
				);
			}
			rec.SetWav(std::move(buf));
		}

		return { rec.GetHandle(), SF2ML_SUCCESS };
	}

	auto SoundFontImpl::AddStereo(const void* wav_data,
								  size_t wav_size,
								  std::string_view name_left,
								  std::string_view name_right,
								  std::optional<Ranges<DWORD>> loop,
								  std::optional<BYTE> root_key,
								  std::optional<CHAR> pitch_correction)
								  -> SF2MLResult<std::pair<SmplHandle, SmplHandle>> {
		auto [ left_smpl, err1] = AddMono(wav_data,
										  wav_size,
										  name_left,
										  loop,
										  root_key,
										  pitch_correction,
										  SampleChannel::Left);
		if (err1) {
			return { { SmplHandle(0), SmplHandle(0) }, err1 };
		}					
		auto [right_smpl, err2] = AddMono(wav_data,
										  wav_size,
										  name_right,
										  loop,
										  root_key,
										  pitch_correction,
										  SampleChannel::Right);
		if (err2) {
			Remove(left_smpl, RemovalMode::Normal);
			return { { SmplHandle(0), SmplHandle(0) }, err2 };
		}
		
		return { { left_smpl, right_smpl }, LinkStereo(left_smpl, right_smpl) };
	}

	auto SoundFontImpl::LinkStereo(SmplHandle left, SmplHandle right) -> SF2MLError {
		SfSample* smpl_left = samples.Get(left);
		SfSample* smpl_right = samples.Get(right);
		if (!smpl_left || !smpl_right) {
			return SF2ML_NO_SUCH_SAMPLE;
		}
		if (smpl_left->GetSampleCount() != smpl_right->GetSampleCount()
			|| smpl_left->GetSampleRate() != smpl_right->GetSampleRate()
			|| smpl_left->GetLoop() != smpl_right->GetLoop()) {
			return SF2ML_BAD_LINK;
		}
		smpl_left->SetSampleMode(leftSample);
		smpl_right->SetSampleMode(rightSample);
		smpl_left->SetLink(right);
		smpl_right->SetLink(left);

		return SF2ML_SUCCESS;
	}

	void SoundFontImpl::Remove(SmplHandle target, RemovalMode rm_mode) {
		SfSample* smpl_ptr = samples.Get(target);
		if (!smpl_ptr) {
			return;
		}

		if (!IsMonoSample(smpl_ptr->GetSampleMode())) {
			if (rm_mode == RemovalMode::Recursive) {
				samples.Remove(*smpl_ptr->GetLink());
			} else {
				SfSample* smpl2_ptr = samples.Get(*smpl_ptr->GetLink());
				if (smpl2_ptr) {
					smpl2_ptr->SetLink(std::nullopt);
				}
			}
		}
		samples.Remove(target);
	}
}