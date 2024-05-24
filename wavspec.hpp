#pragma once

#include <cstdint>

namespace sflib {
	using BYTE  =  uint8_t;
	using WORD  = uint16_t;
	using DWORD = uint32_t;
	using QWORD = uint64_t;
	using CHAR  =   int8_t;
	using SHORT =  int16_t;

	using FOURCC = DWORD;

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
	}
}