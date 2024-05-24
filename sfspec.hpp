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
		SfStartAddrsOffset = 0,
		SfEndAddrsOffset = 1,
		SfStartloopAddrsOffset = 2,
		SfEndloopAddrsOffset = 3,
		SfStartAddrsCoarseOffset = 4,
		SfModLfoToPitch = 5,
		SfVibLfoToPitch = 6,
		SfModEnvToPitch = 7,
		SfInitialFilterFc = 8,
		SfInitialFilterQ = 9,
		SfModLfoToFilterFc = 10,
		SfModEnvToFilterFc = 11,
		SfEndAddrsCoarseOffset = 12,
		SfModLfoToVolume = 13,
		SfChorusEffectsSend = 15,
		SfReverbEffectsSend = 16,
		SfPan = 17,
		SfDelayModLFO = 21,
		SfFreqModLFO = 22,
		SfDelayVibLFO = 23,
		SfFreqVibLFO = 24,
		SfDelayModEnv = 25,
		SfAttackModEnv = 26,
		SfHoldModEnv = 27,
		SfDecayModEnv = 28,
		SfSustainModEnv = 29,
		SfReleaseModEnv = 30,
		SfKeynumToModEnvHold = 31,
		SfKeynumToModEnvDecay = 32,
		SfDelayVolEnv = 33,
		SfAttackVolEnv = 34,
		SfHoldVolEnv = 35,
		SfDecayVolEnv = 36,
		SfSustainVolEnv = 37,
		SfReleaseVolEnv = 38,
		SfKeynumToVolEnvHold = 39,
		SfKeynumToVolEnvDecay = 40,
		SfInstrument = 41,
		SfKeyRange = 43,
		SfVelRange = 44,
		SfStartloopAddrsCoarseOffset = 45,
		SfKeynum = 46,
		SfVelocity = 47,
		SfInitialAttenuation = 48,
		SfEndloopAddrsCoarseOffset = 50,
		SfCoarseTune = 51,
		SfFineTune = 52,
		SfSampleID = 53,
		SfSampleModes = 54,
		SfScaleTuning = 56,
		SfExclusiveClass = 57,
		SfOverridingRootKey = 58,
	};
	
	struct RangesType {
		BYTE by_lo;
		BYTE by_hi;
	};

	union GenAmountType {
		RangesType ranges;
		SHORT sh_amount;
		WORD w_amount;
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

	struct SfbkMap {
		struct Info {
			const MappedChunk<SfVersionTag>* ifil; // the version of the Sound Font RIFF file ; e.g. 2.01
			const MappedChunk<CHAR>* isng; // target Sound Engine ; e.g. EMU8000
			const MappedChunk<CHAR>* inam;  // Sound Font Bank Name ; e.g. General MIDI
			const MappedChunk<CHAR>* irom;  // Sound ROM Name
			const MappedChunk<SfVersionTag>* iver; // Sound ROM Version
			const MappedChunk<CHAR>* icrd;  // date of creation of the bank
			const MappedChunk<CHAR>* ieng;  // sound designers and engineers for the bank
			const MappedChunk<CHAR>* iprd;  // product for which the bank was intended
			const MappedChunk<CHAR>* icop;  // contains any copyright message
			const MappedChunk<CHAR>* icmt;  // contains any comments on the bank
			const MappedChunk<CHAR>* isft;  // the SoundFont tools used to create and alter the bank
		} info;
		struct Sdta {
			const MappedChunk<CHAR>* smpl; // 16-bit samples (when sm24 chunk exists, it is interpreted as upper 16-bit part)
			const MappedChunk<CHAR>* sm24; // lower 8-bit for 24-bit samples
		} sdta;
		struct Pdta {
			const MappedChunk<SfPresetHeader>* phdr;
			const MappedChunk<SfPresetBag>* pbag;
			const MappedChunk<SfModList>* pmod;
			const MappedChunk<SfGenList>* pgen;
			const MappedChunk<SfInst>* inst;
			const MappedChunk<SfInstBag>* ibag;
			const MappedChunk<SfInstModList>* imod;
			const MappedChunk<SfInstGenList>* igen;
			const MappedChunk<SfSample>* shdr;
		} pdta;
	};



	enum SflibError : int {
		SFLIB_SUCCESS = 0,
		SFLIB_FAILED = 1,
		SFLIB_ZSTR_CHECK_FAILED = 2,
		SFLIB_INVALID_CK_SIZE = 3,
		SFLIB_BAD_WAV_DATA = 4,
		SFLIB_UNSUPPORTED_WAV_DATA = 5,
		SFLIB_INCOMPATIBLE_BIT_DEPTH = 6,
		SFLIB_NOT_STEREO_CHANNEL = 7,
		SFLIB_NOT_MONO_CHANNEL = 8,
		SFLIB_NO_SUCH_SAMPLE = 9,
		SFLIB_BAD_LINK = 10,
	};

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

	struct Chunk {
		char ck_id[5];
		std::vector<BYTE> ck_data;
		void Serialize(BYTE* dst) const {
			static_assert(sizeof(BYTE) == sizeof(char));
			DWORD ck_size = ck_data.size();
			std::memcpy(dst, ck_id, sizeof(FOURCC));
			std::memcpy(dst + sizeof(FOURCC), &ck_size, sizeof(DWORD));
			std::memcpy(dst + sizeof(ChunkHead), ck_data.data(), ck_data.size());
		}
	};
}