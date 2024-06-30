#ifndef SF2ML_WAVSPEC_HPP_
#define SF2ML_WAVSPEC_HPP_

#include "sftypes.hpp"
#include <cstdint>
#include <cstddef>

namespace SF2ML {
	enum class SampleBitDepth {
		Signed16, Signed24
	};

	namespace wav {
		enum AudioFormat : WORD {
			AudioFormatPCM = 1,
		};
		enum NumOfChannels : WORD {
			ChannelMono = 1,
			ChannelStereo = 2,
			Channel3 = 3,
			Channel4_Type1 = 4,
			Channel4_Type2 = 5,
			Channel6 = 6,
		};
		struct WaveFmtChunk {
			FOURCC ck_id;
			DWORD ck_size;
			AudioFormat audio_format;
			NumOfChannels num_of_channels;
			DWORD sample_rate;
			DWORD byte_rate;
			WORD block_align;
			WORD bits_per_sample;
		} __attribute__((packed));

		struct WavInfo {
			wav::NumOfChannels num_of_channels;
			uint32_t sample_rate;
			SampleBitDepth bit_depth;
			uint16_t block_align;
			const uint8_t* wav_data;
			uint32_t wav_size;
		};

		SF2MLResult<WavInfo> ValidateWav(const void* data, std::size_t size);
	}
}

#endif