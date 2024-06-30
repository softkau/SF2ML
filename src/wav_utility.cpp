#include <wavspec.hpp>
#include <sfspec.hpp>
#include <sfsample.hpp>
#include "sfmap.hpp"

using namespace SF2ML;

auto SF2ML::wav::ValidateWav(const void* data, std::size_t size) -> SF2ML::SF2MLResult<SF2ML::wav::WavInfo> {
	constexpr DWORD min_wav_file_size = 2*sizeof(ChunkHead) + sizeof(FOURCC) + sizeof(wav::WaveFmtChunk);
	if (size < min_wav_file_size) { // minimum size required for any sane WAV file.
		return { {}, SF2ML_BAD_WAV_DATA };
	}
	const BYTE* by_data = reinterpret_cast<const BYTE*>(data);

	ChunkHead riff;
	std::memcpy(&riff, by_data, sizeof(ChunkHead));

	if (!CheckFOURCC(riff.ck_id, "RIFF")
		|| riff.ck_size < min_wav_file_size - sizeof(ChunkHead)
		|| riff.ck_size > size - sizeof(ChunkHead)
	) {
		return { {}, SF2ML_BAD_WAV_DATA };
	}

	FOURCC fmt;
	std::memcpy(&fmt, by_data + sizeof(ChunkHead), sizeof(FOURCC));
	if (!CheckFOURCC(fmt, "WAVE")) {
		return { {}, SF2ML_BAD_WAV_DATA };
	}

	DWORD offset = sizeof(ChunkHead) + sizeof(FOURCC);
	wav::WaveFmtChunk header;
	std::memcpy(&header, by_data + offset, sizeof(wav::WaveFmtChunk));
	if (!CheckFOURCC(header.ck_id, "fmt ") || header.ck_size != 16) {
		return { {}, SF2ML_BAD_WAV_DATA };
	}
	unsigned ch_nums = static_cast<unsigned>(header.num_of_channels);
	if (header.num_of_channels == wav::Channel4_Type2) {
		ch_nums = 4;
	}
	if (header.byte_rate != (header.bits_per_sample / 8) * header.sample_rate * ch_nums
		|| header.block_align != (header.bits_per_sample / 8) * ch_nums
	) {
		return { {}, SF2ML_BAD_WAV_DATA };
	}

	SF2MLError err = SF2ML_SUCCESS;
	if (!(
		header.audio_format == wav::AudioFormatPCM
		&& (header.num_of_channels == wav::ChannelMono || header.num_of_channels == wav::ChannelStereo)
		&& (header.bits_per_sample == 16 || header.bits_per_sample == 24)
	)) {
		err = SF2ML_UNSUPPORTED_WAV_DATA;
	}

	offset += sizeof(wav::WaveFmtChunk);
	ChunkHead ck;
	std::memcpy(&ck, by_data + offset, sizeof(ChunkHead));
	while (!CheckFOURCC(ck.ck_id, "data")) {
		offset += sizeof(ChunkHead) + ck.ck_size;
		if (offset > riff.ck_size) {
			return { {}, SF2ML_BAD_WAV_DATA };
		}
		std::memcpy(&ck, by_data + offset, sizeof(ChunkHead));
	}

	if (offset + ck.ck_size > riff.ck_size) {
		return { {}, SF2ML_BAD_WAV_DATA };
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