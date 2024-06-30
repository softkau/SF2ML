#ifndef SF2ML_SFSPEC_HPP_
#define SF2ML_SFSPEC_HPP_

#include "sftypes.hpp"
#include <cstdint>
#include <cstring>
#include <vector>
#include <string>
#include <utility>

namespace SF2ML {
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
	
	using SFModulator = WORD;
	using SFTransform = WORD;

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

	constexpr bool IsRamSample(SF2ML::SFSampleLink mode) {
		return !(mode & (SF2ML::RomLeftSample & SF2ML::RomRightSample));
	}
	constexpr bool IsRomSample(SF2ML::SFSampleLink mode) {
		return !IsRamSample(mode);
	}
	constexpr bool IsLeftSample(SF2ML::SFSampleLink mode) {
		return !!(mode & SF2ML::leftSample);
	}
	constexpr bool IsRightSample(SF2ML::SFSampleLink mode) {
		return !!(mode & SF2ML::rightSample);
	}
	constexpr bool IsMonoSample(SF2ML::SFSampleLink mode) {
		return !!(mode & SF2ML::monoSample);
	}
	constexpr bool IsLinkedSample(SF2ML::SFSampleLink mode) {
		return !!(mode & SF2ML::linkedSample);
	}
	
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
	}

	struct ChunkHead {
		FOURCC ck_id;
		DWORD ck_size;
	} __attribute__((packed));
	static_assert(sizeof(ChunkHead) == 8);
}

#endif