#include "sample_manager.hpp"

using namespace sflib;

void SampleManager::SetZeroZone(int count) {
	assert(count >= 46 && count % 2 == 0);
	z_zone = count;
}

DWORD SampleManager::SdtaSize() const {
	DWORD sample16_sz = 0;
	DWORD sample24_sz = 0;
	for (const auto& sample : samples) {
		if (bit_depth == SampleBitDepth::Signed16) {
			sample16_sz += sample.wav_data.size() + z_zone;
		} else {
			sample16_sz += sample.wav_data.size() / 3 * 2 + z_zone;
			sample24_sz += sample.wav_data.size() / 3 + z_zone / 2;
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

DWORD SampleManager::ShdrSize() const {
	return sizeof(ChunkHead) + (samples.Count() + 1) * sizeof(spec::SfSample);
}

SflibError SampleManager::SerializeSDTA(BYTE* dst, BYTE** end_param) const {
	BYTE* pos = dst;
	
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
	if (bit_depth == SampleBitDepth::Signed24) {
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
	assert(sdta_sz + 8 == this->SdtaSize());
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

	const DWORD bytes_per_sample = (bit_depth == SampleBitDepth::Signed16) ? 2 : 3;
	DWORD smpl_idx = 0; // refers to samples in smpl chunk (unit: 16/24 bit sample)
	for (const auto& sample : samples) {
		DWORD smpl_count = sample.wav_data.size() / bytes_per_sample;

		spec::SfSample shdr;
		std::memcpy(shdr.ach_sample_name, sample.sample_name, 20);
		shdr.dw_start        = smpl_idx;
		shdr.dw_end          = smpl_idx + smpl_count;
		shdr.dw_startloop    = smpl_idx + sample.start_loop;
		shdr.dw_endloop      = smpl_idx + sample.end_loop;
		shdr.dw_sample_rate  = sample.sample_rate;
		shdr.by_original_key = sample.root_key;
		shdr.ch_correction   = sample.pitch_correction;
		shdr.w_sample_link   = sample.linked_sample.value;
		shdr.sf_sample_type  = sample.sample_type;

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
	
	if (end_param) {
		*end_param = pos;
	}
	return SFLIB_SUCCESS;
}

SflibResult<SmplHandle> SampleManager::AddMono(
	const void* wav_data,
	std::size_t wav_size,
	const std::string& name,
	std::optional<Ranges<uint32_t>> loop,
	std::optional<uint16_t> root_key,
	std::optional<int16_t> pitch_correction,
	SampleChannel sample_type
) {
	auto [wav_info, err] = SampleManager::ValidateWav(wav_data, wav_size);
	if (err) {
		return { SmplHandle{0}, err };
	}
	if (wav_info.bit_depth != this->bit_depth) {
		return { SmplHandle{0}, SFLIB_INCOMPATIBLE_BIT_DEPTH };
	}
	if (sample_type == SampleChannel::Mono) {
		if (wav_info.num_of_channels != wav::ChannelMono) { // sample_type(arg) is "Mono", but "Stereo" wav data is given
			return { SmplHandle{0}, SFLIB_NOT_MONO_CHANNEL };
		}
	} else {
		if (wav_info.num_of_channels == wav::ChannelMono) { // sample_type(arg) is "Stereo", but "Mono" wav data is given
			return { SmplHandle{0}, SFLIB_NOT_STEREO_CHANNEL };
		}
	}

	const size_t bytes_per_sample = wav_info.bit_depth == SampleBitDepth::Signed16 ? 2 : 3;

	SfSample& rec = samples.NewItem();
	std::memcpy(rec.sample_name, name.c_str(), std::min<std::size_t>(20, name.length()));
	rec.sample_name[20] = 0;
	
	if (loop) {
		rec.start_loop = (*loop).start;
		rec.end_loop = (*loop).end;
	} else {
		rec.start_loop = 0;
		rec.end_loop = wav_info.wav_size / bytes_per_sample;
	}
	if (root_key) {
		rec.root_key = *root_key;
	} else {
		rec.root_key = 60;
	}
	if (pitch_correction) {
		rec.pitch_correction = *pitch_correction;
	} else {
		rec.pitch_correction = 0;
	}

	rec.sample_rate = wav_info.sample_rate;
	if (sample_type == SampleChannel::Mono) {
		rec.wav_data.resize(wav_info.wav_size);
		std::memcpy(rec.wav_data.data(), wav_info.wav_data, wav_info.wav_size);
	} else {
		const size_t data_size = wav_info.wav_size / 2;
		rec.wav_data.resize(data_size);
		const size_t offset = (sample_type == SampleChannel::Left) ? 0 : bytes_per_sample;
		for (size_t i = 0, j = 0; i < data_size; i += bytes_per_sample, j++) {
			std::memcpy(
				&rec.wav_data[i],
				&wav_info.wav_data[j * wav_info.block_align + offset],
				bytes_per_sample
			);
		}
	}
	rec.sample_type = monoSample;
	rec.linked_sample = SmplHandle{0};
	return { rec.GetHandle(), SFLIB_SUCCESS };
}

SflibResult<std::pair<SmplHandle, SmplHandle>> SampleManager::AddStereo(
	const void* wav_data,
	size_t wav_size,
	const std::string& name_left,
	const std::string& name_right,
	std::optional<Ranges<uint32_t>> loop,
	std::optional<uint16_t> root_key,
	std::optional<int16_t> pitch_correction
) {
	auto [ left_smpl, err1] = AddMono(wav_data, wav_size, name_left, loop, root_key, pitch_correction, SampleChannel::Left);
	auto [right_smpl, err2] = AddMono(wav_data, wav_size, name_right, loop, root_key, pitch_correction, SampleChannel::Right);
	if (err1 | err2) {
		SflibError err = err1;
		if (err1 == SFLIB_SUCCESS) {
			err = err2;
			Remove( left_smpl, RemovalMode::Normal);
		}
		if (err2 == SFLIB_SUCCESS) {
			err = err1;
			Remove(right_smpl, RemovalMode::Normal);
		}
		return { {SmplHandle(0), SmplHandle(0)}, err };
	}
	return { { left_smpl, right_smpl }, LinkStereo(left_smpl, right_smpl) };
}

void SampleManager::Remove(SmplHandle target, RemovalMode rm_mode) {
	SfSample* smpl_ptr = samples.Get(target);
	if (!smpl_ptr) {
		return;
	}

	if (smpl_ptr->sample_type & (leftSample | rightSample)) {
		if (rm_mode == RemovalMode::Recursive) {
			samples.Remove(smpl_ptr->linked_sample);
		} else {
			SfSample* smpl2_ptr = samples.Get(smpl_ptr->linked_sample);
			if (smpl2_ptr) {
				smpl2_ptr->SetLink(std::nullopt);
			}
		}
	}
	samples.Remove(target);
}

SflibError SampleManager::LinkStereo(SmplHandle left, SmplHandle right) {
	SfSample* smpl_left = samples.Get(left);
	SfSample* smpl_right = samples.Get(right);
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
			.bit_depth = header.bits_per_sample == 16 ? SampleBitDepth::Signed16 : SampleBitDepth::Signed24,
			.block_align = header.block_align,
			.wav_data = by_data + offset + sizeof(ChunkHead),
			.wav_size = ck.ck_size
		}, err
	};
}

std::optional<SampleID> SampleManager::GetSampleID(SmplHandle target) const {
	return samples.GetID(target);
}
