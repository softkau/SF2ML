#pragma once

#include <cstdint>
#include <cstring>
#include <vector>
#include <string>
#include <utility>

namespace sflib {
	using BYTE  =  uint8_t;
	using WORD  = uint16_t;
	using DWORD = uint32_t;
	using QWORD = uint64_t;
	using CHAR  =   int8_t;
	using SHORT =  int16_t;

	using FOURCC = DWORD;

	using SFModulator = WORD;
	using SFTransform = WORD;

	enum SFGenerator : WORD {
		SfGenStartAddrsOffset = 0,
		SfGenEndAddrsOffset = 1,
		SfGenStartloopAddrsOffset = 2,
		SfGenEndloopAddrsOffset = 3,
		SfGenStartAddrsCoarseOffset = 4,
		SfGenModLfoToPitch = 5,
		SfGenVibLfoToPitch = 6,
		SfGenModEnvToPitch = 7,
		SfGenInitialFilterFc = 8,
		SfGenInitialFilterQ = 9,
		SfGenModLfoToFilterFc = 10,
		SfGenModEnvToFilterFc = 11,
		SfGenEndAddrsCoarseOffset = 12,
		SfGenModLfoToVolume = 13,
		SfGenChorusEffectsSend = 15,
		SfGenReverbEffectsSend = 16,
		SfGenPan = 17,
		SfGenDelayModLFO = 21,
		SfGenFreqModLFO = 22,
		SfGenDelayVibLFO = 23,
		SfGenFreqVibLFO = 24,
		SfGenDelayModEnv = 25,
		SfGenAttackModEnv = 26,
		SfGenHoldModEnv = 27,
		SfGenDecayModEnv = 28,
		SfGenSustainModEnv = 29,
		SfGenReleaseModEnv = 30,
		SfGenKeynumToModEnvHold = 31,
		SfGenKeynumToModEnvDecay = 32,
		SfGenDelayVolEnv = 33,
		SfGenAttackVolEnv = 34,
		SfGenHoldVolEnv = 35,
		SfGenDecayVolEnv = 36,
		SfGenSustainVolEnv = 37,
		SfGenReleaseVolEnv = 38,
		SfGenKeynumToVolEnvHold = 39,
		SfGenKeynumToVolEnvDecay = 40,
		SfGenInstrument = 41,
		SfGenKeyRange = 43,
		SfGenVelRange = 44,
		SfGenStartloopAddrsCoarseOffset = 45,
		SfGenKeynum = 46,
		SfGenVelocity = 47,
		SfGenInitialAttenuation = 48,
		SfGenEndloopAddrsCoarseOffset = 50,
		SfGenCoarseTune = 51,
		SfGenFineTune = 52,
		SfGenSampleID = 53,
		SfGenSampleModes = 54,
		SfGenScaleTuning = 56,
		SfGenExclusiveClass = 57,
		SfGenOverridingRootKey = 58,
		SfGenEndOper = 60,
	};

	enum SFSampleLink : WORD {
		monoSample = 1,
		rightSample = 2,
		leftSample = 4,
		linkedSample = 8,
		RomMonoSample = 0x8001,
		RomRightSample = 0x8002,
		RomLeftSample = 0x8004,
		RomLinkedSample = 0x8008
	};
	
	namespace spec {
		struct RangesType {
			BYTE by_lo;
			BYTE by_hi;
		};

		union GenAmountType {
			RangesType ranges;
			SHORT sh_amount;
			WORD w_amount;
		};

		static_assert(sizeof(SFSampleLink) == 2);
		struct SfVersionTag {
			WORD w_major;
			WORD w_minor;
		} __attribute__((packed));

		struct SfPresetHeader {
			CHAR ach_preset_name[20];
			WORD w_preset;
			WORD w_bank;
			WORD w_preset_bag_ndx;
			DWORD dw_library;
			DWORD dw_genre;
			DWORD dw_morphology;
		} __attribute__((packed));

		struct SfPresetBag {
			WORD w_gen_ndx;
			WORD w_mod_ndx;
		} __attribute__((packed));

		struct SfModList {
			SFModulator sf_mod_src_oper;
			SFGenerator sf_mod_dest_oper;
			SHORT mod_amount;
			SFModulator sf_mod_amt_src_oper;
			SFTransform sf_mod_trans_oper;
		} __attribute__((packed));

		struct SfGenList {
			SFGenerator sf_gen_oper;
			GenAmountType gen_amount;
		} __attribute__((packed));

		struct SfInst {
			CHAR ach_inst_name[20];
			WORD w_inst_bag_ndx;
		} __attribute__((packed));

		struct SfInstBag {
			WORD w_inst_gen_ndx;
			WORD w_inst_mod_ndx;
		} __attribute__((packed));

		struct SfInstModList {
			SFModulator sf_mod_src_oper;
			SFGenerator sf_mod_dest_oper;
			SHORT mod_amount;
			SFModulator sf_mod_amt_src_oper;
			SFTransform sf_mod_trans_oper;
		} __attribute__((packed));

		struct SfInstGenList {
			SFGenerator sf_gen_oper;
			GenAmountType gen_amount;
		} __attribute__((packed));

		struct SfSample {
			CHAR ach_sample_name[20];
			DWORD dw_start;
			DWORD dw_end;
			DWORD dw_startloop;
			DWORD dw_endloop;
			DWORD dw_sample_rate;
			BYTE by_original_key;
			CHAR ch_correction;
			WORD w_sample_link;
			SFSampleLink sf_sample_type;
		} __attribute__((packed));

		template <class RecordType> 
		struct MappedChunk {
			FOURCC ck_id;
			DWORD ck_size;
			RecordType ck_data[];
		} __attribute__((packed));
	}

	enum [[nodiscard]] SflibError : int {
		SFLIB_SUCCESS = 0,
		SFLIB_FAILED,
		SFLIB_ZSTR_CHECK_FAILED,
		SFLIB_INVALID_CK_SIZE,
		SFLIB_BAD_WAV_DATA,
		SFLIB_UNSUPPORTED_WAV_DATA,
		SFLIB_INCOMPATIBLE_BIT_DEPTH,
		SFLIB_NOT_STEREO_CHANNEL,
		SFLIB_NOT_MONO_CHANNEL,
		SFLIB_NO_SUCH_SAMPLE,
		SFLIB_BAD_LINK,
		SFLIB_NO_SUCH_INSTRUMENT,
		SFLIB_MISSING_TERMINAL_RECORD,
		SFLIB_EMPTY_CHUNK,
		SFLIB_UNIMPLEMENTED,
		SFLIB_END_OF_ERRCODE
	};

	constexpr const char* SflibErrorStr[] = {
		"SFLIB_SUCCESS",
		"SFLIB_FAILED",
		"SFLIB_ZSTR_CHECK_FAILED",
		"SFLIB_INVALID_CK_SIZE",
		"SFLIB_BAD_WAV_DATA",
		"SFLIB_UNSUPPORTED_WAV_DATA",
		"SFLIB_INCOMPATIBLE_BIT_DEPTH",
		"SFLIB_NOT_STEREO_CHANNEL",
		"SFLIB_NOT_MONO_CHANNEL",
		"SFLIB_NO_SUCH_SAMPLE",
		"SFLIB_BAD_LINK",
		"SFLIB_NO_SUCH_INSTRUMENT",
		"SFLIB_MISSING_TERMINAL_RECORD",
		"SFLIB_EMPTY_CHUNK",
		"SFLIB_UNIMPLEMENTED",
	}; static_assert(SFLIB_END_OF_ERRCODE == sizeof(SflibErrorStr) / sizeof(const char*));

	template <class T>
	struct SflibResult {
		T value;
		SflibError error;
	};

	inline bool CheckFOURCC(FOURCC fourcc, const char* str) {
		char byte_fourcc[4];
		std::memcpy(byte_fourcc, &fourcc, sizeof(byte_fourcc));

		return byte_fourcc[0] == str[0] &&
			   byte_fourcc[1] == str[1] &&
			   byte_fourcc[2] == str[2] &&
			   byte_fourcc[3] == str[3];
	}

	inline std::string FOURCCtoString(FOURCC fourcc) {
		std::string res = "xxxx";
		memcpy(res.data(), reinterpret_cast<const char*>(&fourcc), sizeof(DWORD));
		return res;
	}

	struct ChunkHead {
		FOURCC ck_id;
		DWORD ck_size;
	};
	static_assert(sizeof(ChunkHead) == 8);

	inline std::pair<DWORD, SflibError> ReadChunkHead(ChunkHead& dst, const BYTE* buf, DWORD buf_size, DWORD offset) {
		if (offset + sizeof(ChunkHead) > buf_size) {
			return { offset, SFLIB_FAILED };
		}
		std::memcpy(&dst, buf + offset, sizeof(ChunkHead));
		if (offset + sizeof(ChunkHead) + dst.ck_size > buf_size || dst.ck_size % 2 != 0) {
			return { offset, SFLIB_FAILED };
		}
		offset += sizeof(ChunkHead) + dst.ck_size;
		return { offset, SFLIB_SUCCESS };
	}

	template <typename T>
	struct Ranges {
		T start;
		T end;
	};

	enum class RemovalMode {
		Restrict, Cascade, Force, Recursive, Normal
	};

	enum class SampleChannel {
		Mono, Left, Right,
	};
}