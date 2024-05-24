#include "sample_manager.hpp"

using namespace sflib;

SampleManager::SampleManager(const BYTE* sdta, DWORD sdta_size, const BYTE* pdta, DWORD pdta_size) {
	if (sdta_size < 4 || pdta_size < 4) {
		status = SFLIB_FAILED;
		return;
	}

	FOURCC fourcc;
	std::memcpy(&fourcc, pdta, sizeof(fourcc));
	if (!CheckFOURCC(fourcc, "pdta")) {
		status = SFLIB_FAILED;
		return;
	}
	std::memcpy(&fourcc, sdta, sizeof(fourcc));
	if (!CheckFOURCC(fourcc, "sdta")) {
		status = SFLIB_FAILED;
		return;
	}

	ChunkHead ck_head;
	DWORD offset = sizeof(fourcc);
	for (int i = 0; i < 9; i++) {
		auto [next, err] = ReadChunkHead(ck_head, pdta, pdta_size, offset);
		if (err) {
			status = err;
			return;
		}
		if (i != 8) {
			offset = next;
		}
	}

	if (!CheckFOURCC(ck_head.ck_id, "shdr")) {
		status = SFLIB_FAILED;
		return;
	}
	const BYTE* shdr = pdta + offset + sizeof(ChunkHead);
	const DWORD shdr_size = ck_head.ck_size;

	if (shdr_size % sizeof(SfSample) != 0) {
		status = SFLIB_INVALID_CK_SIZE;
		return;
	}

	const BYTE* smpl = nullptr;
	DWORD smpl_size = 0;
	const BYTE* sm24 = nullptr;
	DWORD sm24_size = 0;

	offset = sizeof(fourcc);
	while (offset < sdta_size) {
		auto [next, err] = ReadChunkHead(ck_head, sdta, sdta_size, offset);
		if (err) {
			status = err;
			return;
		}

		if (CheckFOURCC(ck_head.ck_id, "smpl")) {
			smpl = sdta + offset + sizeof(ChunkHead);
			smpl_size = ck_head.ck_size;
		} else if (CheckFOURCC(ck_head.ck_id, "sm24")) {
			sm24 = sdta + offset + sizeof(ChunkHead);
			sm24_size = ck_head.ck_size;
		}
		offset = next;
	}

	if (smpl) {
		sample_bitdepth = 16;
		if (sm24 && ((smpl_size/2 + 1) >> 1 << 1) == sm24_size) {
			sample_bitdepth = 24;
		}
	}

	std::size_t sample_count = shdr_size / sizeof(SfSample);
	if (sample_count == 0) {
		status = SFLIB_FAILED;
		return;
	}
	sample_count--;

	// populate shdr chunk
	for (SampleID id = 0; id < sample_count; id++) {
		SfSample cur_shdr;
		std::memcpy(&cur_shdr, shdr + id * sizeof(SfSample), sizeof(SfSample));

		SampleData smpl_data;
		std::memcpy(smpl_data.sample_name, cur_shdr.ach_sample_name, 20);
		smpl_data.sample_name[20] = 0;

		smpl_data.sample_rate = cur_shdr.dw_sample_rate;
		smpl_data.start_loop = cur_shdr.dw_startloop - cur_shdr.dw_start;
		smpl_data.end_loop = cur_shdr.dw_endloop - cur_shdr.dw_start;
		smpl_data.root_key = cur_shdr.by_original_key;
		smpl_data.pitch_correction = cur_shdr.ch_correction;
		smpl_data.linked_sample.key = 0;
		smpl_data.sample_type = cur_shdr.sf_sample_type;
		if (smpl_data.sample_type & (leftSample | rightSample)) {
			// when loading from file, sample id == sample handle
			smpl_data.linked_sample.key = cur_shdr.w_sample_link;
		}
		
		if (smpl_data.sample_type & (RomLeftSample & RomRightSample)) {
			logger(std::string("ROM sample detected: <") + smpl_data.sample_name + "> currently unimplemeneted.\n");

		} else if (smpl && cur_shdr.dw_start < cur_shdr.dw_end && cur_shdr.dw_end <= smpl_size / 2) {
			if (sample_bitdepth == 16) {
				smpl_data.wav_data.resize((cur_shdr.dw_end - cur_shdr.dw_start) * 2);
				std::memcpy(smpl_data.wav_data.data(), &smpl[cur_shdr.dw_start * 2], smpl_data.wav_data.size());
			} else if (sample_bitdepth == 24) {
				smpl_data.wav_data.resize((cur_shdr.dw_end - cur_shdr.dw_start) * 3);
				for (size_t i = cur_shdr.dw_start; i < cur_shdr.dw_end; i++) {
					smpl_data.wav_data[3 * i + 0] = sm24[i];
					smpl_data.wav_data[3 * i + 1] = smpl[2 * i + 0];
					smpl_data.wav_data[3 * i + 2] = smpl[2 * i + 1];
				}
			}
		} else {
			logger("Sample #" + std::to_string(id) + " data is corrupted.\n");
		}

		SfHandle handle = samples.EmplaceBack(std::move(smpl_data));
		smpl_index.emplace(std::string(smpl_data.sample_name), handle);
	}
	status = SFLIB_SUCCESS;
}

void SampleManager::SetZeroZone(int count) {
	assert(count >= 46 && count % 2 == 0);
	z_zone = count;
}

DWORD SampleManager::ChunkSize() const {
	DWORD sample_count = 0;
	DWORD sample16_sz = 0;
	DWORD sample24_sz = 0;
	for (const auto& sample : samples) {
		if (sample_bitdepth == 16) {
			sample16_sz += sample.wav_data.size() + z_zone;
		} else {
			sample16_sz += sample.wav_data.size() / 3 * 2 + z_zone;
			sample24_sz += sample.wav_data.size() / 3 + z_zone / 2;
		}
		sample_count++;
	}

	DWORD shdr_size = sizeof(ChunkHead) + (sample_count + 1) * sizeof(SfSample);
	DWORD smpl_size = sizeof(ChunkHead) + sample16_sz;
	DWORD sm24_size = (sample_bitdepth == 24) ? sizeof(ChunkHead) + sample24_sz : 0;
	if (sm24_size % 2 == 1) {
		sm24_size++;
	}

	DWORD sdta_size = smpl_size + sm24_size;
	return sdta_size + shdr_size;
}

SflibError SampleManager::SerializeSDTA(BYTE* dst, BYTE** end_param) const {
	BYTE* pos = dst;
	
	// serialize ./sdta
	BYTE* const sdta_head = pos;
	std::memcpy(sdta_head, "sdta", 4);
	pos += 8;
	
	// serialize ./sdta/smpl
	BYTE* const smpl_head = pos;
	std::memcpy(smpl_head, "smpl", 4);
	pos += 8;
	if (sample_bitdepth == 16) {
		for (const auto& sample : samples) {
			std::memcpy(pos, sample.wav_data.data(), sample.wav_data.size());
			pos += sample.wav_data.size();
			std::memset(pos, 0, z_zone);
			pos += z_zone;
		}
	} else {
		for (const auto& sample : samples) {
			DWORD smpl_count = sample.wav_data.size() / 3;
			for (DWORD idx = 0; idx < smpl_count; idx++) {
				pos[0] = sample.wav_data[3 * idx + 1];
				pos[1] = sample.wav_data[3 * idx + 2];
				pos += 2;
			}
			std::memset(pos, 0, z_zone);
			pos += z_zone;
		}
	}
	DWORD smpl_sz = pos - smpl_head - 8;
	std::memcpy(smpl_head + 4, &smpl_sz, sizeof(smpl_sz));

	// serialize ./sdta/sm24
	if (sample_bitdepth == 24) {
		BYTE* const sm24_head = pos;
		std::memcpy(sm24_head, "sm24", 4);
		pos += 8;
		for (const auto& sample : samples) {
			DWORD smpl_count = sample.wav_data.size() / 3;
			for (DWORD idx = 0; idx < smpl_count; idx++) {
				*pos = sample.wav_data[3 * idx + 0];
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

	if (end_param) {
		*end_param = pos;
	}
	return SFLIB_SUCCESS;
}

SflibError SampleManager::SerializeSHDR(BYTE* dst, BYTE** end_param) const {
	BYTE* pos = dst;

	BYTE* const shdr_head = pos;
	std::memcpy(shdr_head, "shdr", 4);
	pos += 8;

	const DWORD bytes_per_sample = (sample_bitdepth == 16) ? 2 : 3;
	DWORD smpl_idx = 0; // refers to samples in smpl chunk (unit: 16/24 bit sample)
	for (const auto& sample : samples) {
		DWORD smpl_count = sample.wav_data.size() / bytes_per_sample;

		SfSample shdr;
		std::memcpy(shdr.ach_sample_name, sample.sample_name, 20);
		shdr.dw_start        = smpl_idx;
		shdr.dw_end          = smpl_idx + smpl_count;
		shdr.dw_startloop    = smpl_idx + sample.start_loop;
		shdr.dw_endloop      = smpl_idx + sample.end_loop;
		shdr.dw_sample_rate  = sample.sample_rate;
		shdr.by_original_key = sample.root_key;
		shdr.ch_correction   = sample.pitch_correction;
		shdr.w_sample_link   = sample.linked_sample.key;
		shdr.sf_sample_type  = sample.sample_type;

		smpl_idx += smpl_count + z_zone / 2; // update sample starting point
		std::memcpy(pos, &shdr, sizeof(shdr));
		pos += sizeof(shdr);
	}
	// Append EOS (End Of Sample) Entry
	SfSample eos { "EOS" };
	std::memcpy(pos, &eos, sizeof(eos));

	DWORD shdr_sz = pos - shdr_head - 8;
	std::memcpy(shdr_head + 4, &shdr_sz, sizeof(shdr_sz));
	
	if (end_param) {
		*end_param = pos;
	}
	return SFLIB_SUCCESS;
}

SflibResult<SfHandle> SampleManager::AddMono(const void* wav_data, std::size_t wav_size,
	std::optional<std::string> name,
	std::optional<Ranges<uint32_t>> loop,
	std::optional<uint16_t> root_key,
	std::optional<int16_t> pitch_correction,
	SampleChannel sample_type
) {
	auto [wav_info, err] = SampleManager::ValidateWav(wav_data, wav_size);
	if (err) {
		return { SfHandle{0}, err };
	}
	if (wav_info.bit_depth != this->sample_bitdepth) {
		return { SfHandle{0}, SFLIB_INCOMPATIBLE_BIT_DEPTH };
	}
	if (sample_type == SampleChannel::Mono) {
		if (wav_info.num_of_channels != wav::ChannelMono) {
			return { SfHandle{0}, SFLIB_NOT_MONO_CHANNEL };
		}
	} else {
		if (wav_info.num_of_channels == wav::ChannelMono) {
			return { SfHandle{0}, SFLIB_NOT_STEREO_CHANNEL };
		}
	}
	
	SampleData sdata;
	if (name) {
		std::memcpy(sdata.sample_name, (*name).c_str(), std::min<std::size_t>(20, (*name).length()));
		sdata.sample_name[20] = 0;
	} else {
		std::strcpy(sdata.sample_name, "UNTITLED SAMPLE");
	}
	if (loop) {
		sdata.start_loop = (*loop).start;
		sdata.end_loop = (*loop).end;
	} else {
		sdata.start_loop = 0;
		sdata.end_loop = wav_info.wav_size / (wav_info.bit_depth >> 3);
	}
	if (root_key) {
		sdata.root_key = *root_key;
	} else {
		sdata.root_key = 60;
	}
	if (pitch_correction) {
		sdata.pitch_correction = *pitch_correction;
	} else {
		sdata.pitch_correction = 0;
	}

	sdata.sample_rate = wav_info.sample_rate;
	if (sample_type == SampleChannel::Mono) {
		sdata.wav_data.resize(wav_info.wav_size);
		std::memcpy(sdata.wav_data.data(), wav_info.wav_data, wav_info.wav_size);
	} else {
		const size_t data_size = wav_info.wav_size / 2;
		const size_t bytes_per_sample = wav_info.bit_depth / 8;
		sdata.wav_data.resize(data_size);
		for (size_t i = 0, j = 0; i < data_size; i += bytes_per_sample, j++) { // left sample
			size_t offset = (sample_type == SampleChannel::Left) ? 0 : bytes_per_sample;
			std::memcpy(&sdata.wav_data[i],
				&wav_info.wav_data[j * wav_info.block_align + offset], bytes_per_sample);
		}
	}
	sdata.sample_type = monoSample;
	sdata.linked_sample = SfHandle{0};
	SfHandle handle = this->samples.EmplaceBack(std::move(sdata));
	smpl_index.emplace(std::string(sdata.sample_name), handle);
	return { handle, SFLIB_SUCCESS };
}

SflibResult<SfHandle> SampleManager::AddStereo(const void* wav_data, size_t wav_size,
	std::optional<std::string> name,
	std::optional<Ranges<uint32_t>> loop,
	std::optional<uint16_t> root_key,
	std::optional<int16_t> pitch_correction
) {
	std::string base_name = name.has_value() ? (*name).substr(0, 18) : "DUMMY";
	auto [ left_smpl, err1] = AddMono(wav_data, wav_size, base_name + "_L", loop, root_key, pitch_correction, SampleChannel::Left);
	auto [right_smpl, err2] = AddMono(wav_data, wav_size, base_name + "_R", loop, root_key, pitch_correction, SampleChannel::Right);
	if (err1 | err2) {
		SflibError err;
		if (err1 == SFLIB_SUCCESS) {
			err = err2;
			Remove( left_smpl, RemovalMode::Force);
		}
		if (err2 == SFLIB_SUCCESS) {
			err = err1;
			Remove(right_smpl, RemovalMode::Force);
		}
		return { SfHandle{0}, err };
	}
	return { left_smpl, LinkStereo(left_smpl, right_smpl) };
}

SflibError SampleManager::Remove(SfHandle target, RemovalMode mode) {
	SampleData* smpl_ptr = samples.Get(target);
	if (!smpl_ptr) {
		return SFLIB_FAILED;
	}

	SampleData* smpl2_ptr = nullptr;
	if (smpl_ptr->sample_type & (leftSample | rightSample)) {
		smpl2_ptr = samples.Get(smpl_ptr->linked_sample);
		if (!smpl2_ptr) {
			return SFLIB_FAILED;
		}
	}

	if (!smpl2_ptr) {
		auto ref = referenced.find(target);
		switch (mode) {
		case RemovalMode::Restrict:
		case RemovalMode::Cascade:
			if (ref == referenced.end()) {
				samples.Remove(target);
				std::erase_if(smpl_index, [target](const auto& item) { return item.second == target; });
				return SFLIB_SUCCESS;
			} else {
				return SFLIB_FAILED;
			}
		case RemovalMode::Force:
			samples.Remove(target);
			std::erase_if(smpl_index, [target](const auto& item) { return item.second == target; });
			return SFLIB_SUCCESS;
		}
	} else {
		const SfHandle target2 = smpl_ptr->linked_sample;
		auto ref1 = referenced.find(target);
		auto ref2 = referenced.find(target2);
		switch (mode) {
		case RemovalMode::Restrict:
		case RemovalMode::Cascade:
			if (ref1 == referenced.end() && ref2 == referenced.end()) {
				samples.Remove(target);
				samples.Remove(smpl_ptr->linked_sample);
				std::erase_if(smpl_index, [target](const auto& item) { return item.second == target; });
				std::erase_if(smpl_index, [target2](const auto& item) { return item.second == target2; });
				return SFLIB_SUCCESS;
			} else {
				return SFLIB_FAILED;
			}
		case RemovalMode::Force:
			samples.Remove(target);
			samples.Remove(smpl_ptr->linked_sample);
			std::erase_if(smpl_index, [target](const auto& item) { return item.second == target; });
			std::erase_if(smpl_index, [target2](const auto& item) { return item.second == target2; });
			return SFLIB_SUCCESS;
		}
	}
	return SFLIB_FAILED;
}

SflibError SampleManager::LinkStereo(SfHandle left, SfHandle right) {
	SampleData* smpl_left = samples.Get(left);
	SampleData* smpl_right = samples.Get(right);
	if (!smpl_left || !smpl_right) {
		return SFLIB_NO_SUCH_SAMPLE;
	}
	if (smpl_left->wav_data.size() != smpl_right->wav_data.size()
		|| smpl_left->sample_rate != smpl_right->sample_rate
		|| smpl_left->start_loop != smpl_right->start_loop
		|| smpl_left->end_loop != smpl_right->end_loop
	) {
		return SFLIB_BAD_LINK;
	}
	smpl_left->sample_type = leftSample;
	smpl_right->sample_type = rightSample;
	smpl_left->linked_sample = right;
	smpl_right->linked_sample = left;
	
	return SFLIB_SUCCESS;
}

void SampleManager::SetName(SfHandle target, const std::string& value) {
	if (auto smpl = samples.Get(target)) {
		std::memcpy(smpl->sample_name, value.c_str(), 20);
	}
}
std::string SampleManager::GetName(SfHandle target) const {
	if (auto smpl = samples.Get(target)) {
		return smpl->sample_name;
	}
	return "";
}
void SampleManager::SetLoopPoint(SfHandle target, uint32_t start, uint32_t end) {
	if (auto smpl = samples.Get(target)) {
		smpl->start_loop = start;
		smpl->end_loop = end;
	}
}
Ranges<uint32_t> SampleManager::GetLoopPoint(SfHandle target) const {
	if (auto smpl = samples.Get(target)) {
		return { smpl->start_loop, smpl->end_loop };
	}
	return {};
}
void SampleManager::SetRootKey(SfHandle target, uint16_t value) {
	if (auto smpl = samples.Get(target)) {
		smpl->root_key = value;
	}
}
uint16_t SampleManager::GetRootKey(SfHandle target) const {
	return samples.Get(target)->root_key;
}
void SampleManager::SetPitchCorrection(SfHandle target, int16_t value) {
	samples.Get(target)->pitch_correction = value;
}
int16_t SampleManager::GetPitchCorrection(SfHandle target) const {
	return samples.Get(target)->pitch_correction;
}
int32_t SampleManager::GetSampleRate(SfHandle target) const {
	return samples.Get(target)->sample_rate;
}
SfHandle SampleManager::GetLinkedSample(SfHandle target) const {
	return samples.Get(target)->linked_sample;
}
bool SampleManager::IsMono(SfHandle target) const {
	return (samples.Get(target)->sample_type & monoSample) != 0;
}
bool SampleManager::IsLeftChannel(SfHandle target) const {
	return (samples.Get(target)->sample_type & leftSample) != 0;
}
bool SampleManager::IsRightChannel(SfHandle target) const {
	return (samples.Get(target)->sample_type & rightSample) != 0;
}
bool SampleManager::IsRomSample(SfHandle target) const {
	return (samples.Get(target)->sample_type & (RomLeftSample & RomRightSample)) != 0;
}

SflibResult<WavInfo> SampleManager::ValidateWav(const void* data, size_t size) {
	constexpr DWORD min_wav_file_size = 2*sizeof(ChunkHead) + sizeof(FOURCC) + sizeof(wav::WaveFmtChunk);
	if (size < min_wav_file_size) { // minimum size required for any sane WAV file.
		return { {}, SFLIB_BAD_WAV_DATA };
	}
	const BYTE* by_data = reinterpret_cast<const BYTE*>(data);

	ChunkHead riff;
	std::memcpy(&riff, by_data, sizeof(ChunkHead));

	if (!CheckFOURCC(riff.ck_id, "RIFF")
		|| riff.ck_size < min_wav_file_size - sizeof(ChunkHead)
		|| riff.ck_size > size - sizeof(ChunkHead)
	) {
		return { {}, SFLIB_BAD_WAV_DATA };
	}

	FOURCC fmt;
	std::memcpy(&fmt, by_data + sizeof(ChunkHead), sizeof(FOURCC));
	if (!CheckFOURCC(fmt, "WAVE")) {
		return { {}, SFLIB_BAD_WAV_DATA };
	}

	DWORD offset = sizeof(ChunkHead) + sizeof(FOURCC);
	wav::WaveFmtChunk header;
	std::memcpy(&header, by_data + offset, sizeof(wav::WaveFmtChunk));
	if (!CheckFOURCC(header.ck_id, "fmt ") || header.ck_size != 16) {
		return { {}, SFLIB_BAD_WAV_DATA };
	}
	unsigned ch_nums = static_cast<unsigned>(header.num_of_channels);
	if (header.num_of_channels == wav::Channel4_Type2) {
		ch_nums = 4;
	}
	if (header.byte_rate != (header.bits_per_sample / 8) * header.sample_rate * ch_nums
		|| header.block_align != (header.bits_per_sample / 8) * ch_nums
	) {
		return { {}, SFLIB_BAD_WAV_DATA };
	}

	SflibError err = SFLIB_SUCCESS;
	if (!(
		header.audio_format == wav::AudioFormatPCM
		&& (header.num_of_channels == wav::ChannelMono || header.num_of_channels == wav::ChannelStereo)
		&& (header.bits_per_sample == 16 || header.bits_per_sample == 24)
	)) {
		err = SFLIB_UNSUPPORTED_WAV_DATA;
	}

	offset += sizeof(wav::WaveFmtChunk);
	ChunkHead ck;
	std::memcpy(&ck, by_data + offset, sizeof(ChunkHead));
	while (!CheckFOURCC(ck.ck_id, "data")) {
		offset += sizeof(ChunkHead) + ck.ck_size;
		if (offset > riff.ck_size) {
			return { {}, SFLIB_BAD_WAV_DATA };
		}
		std::memcpy(&ck, by_data + offset, sizeof(ChunkHead));
	}

	if (offset + ck.ck_size > riff.ck_size) {
		return { {}, SFLIB_BAD_WAV_DATA };
	}

	return {
		WavInfo {
			.num_of_channels = header.num_of_channels,
			.sample_rate = header.sample_rate,
			.bit_depth = header.bits_per_sample,
			.block_align = header.block_align,
			.wav_data = by_data + offset + sizeof(ChunkHead),
			.wav_size = ck.ck_size
		}, err
	};
}

SflibError SampleManager::AddRef(SfHandle smpl, InstID inst) {
	if (!samples.Get(smpl)) {
		return SFLIB_NO_SUCH_SAMPLE;
	}
	referenced.emplace(smpl, inst);
	return SFLIB_SUCCESS;
}

SflibError SampleManager::RemoveRef(SfHandle smpl, InstID inst) {
	auto [first, last] = referenced.equal_range(smpl);
	auto it = std::find_if(first, last, [inst](auto& pair) { return pair.second == inst; });
	if (it == last) {
		return SFLIB_FAILED;
	}
	referenced.erase(it);
	return SFLIB_SUCCESS;
}
