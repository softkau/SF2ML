#pragma once

#include "info_manager.hpp"
#include "sample_manager.hpp"
#include "instrument_manager.hpp"
#include "preset_manager.hpp"
#include "sfmap.hpp"
#include <memory>

namespace sflib {

	class SountFont2 {
	public:
		SountFont2(std::istream& is) {
			is.seekg(0, std::ios::end);
			DWORD sz = is.tellg();
			is.seekg(0, std::ios::beg);
			sz -= is.tellg();
			std::vector<BYTE> buf(sz);
			is.read((char*)buf.data(), sz);
			init(buf.data(), sz);
		}
		void init(const BYTE* buf, std::size_t buf_size) {
			ChunkHead ck_head;
			auto [riff_next, riff_err] = ReadChunkHead(ck_head, buf, buf_size, 0);
			if (riff_err) {
				status = riff_err;
				return;
			}
			if (!CheckFOURCC(ck_head.ck_id, "RIFF")) {
				status = SFLIB_FAILED;
				return;
			}

			const BYTE* riff_buf = buf + sizeof(ChunkHead);
			const std::size_t riff_size = ck_head.ck_size;

			SfbkMap sfbk_ptr;
			if (auto err = GetSfbkMap(sfbk_ptr, riff_buf, riff_size)) {
				status = err;
				return;
			}

			FOURCC sfbk_fourcc;
			std::memcpy(&sfbk_fourcc, riff_buf, sizeof(FOURCC));
			if (!CheckFOURCC(sfbk_fourcc, "sfbk")) {
				status = SFLIB_FAILED;
				return;
			}
			FOURCC info_offset = sizeof(FOURCC);

			ChunkHead info_head;
			auto [sdta_offset, info_err] = ReadChunkHead(info_head, riff_buf, riff_size, info_offset);
			if (info_err) {
				status = info_err;
				return;
			}

			ChunkHead sdta_head;
			auto [pdta_offset, sdta_err] = ReadChunkHead(sdta_head, riff_buf, riff_size, sdta_offset);
			if (sdta_err) {
				status = sdta_err;
				return;
			}

			ChunkHead pdta_head;
			auto [last_offset, pdta_err] = ReadChunkHead(pdta_head, riff_buf, riff_size, pdta_offset);
			if (pdta_err) {
				status = pdta_err;
				return;
			}

			if (!CheckFOURCC(info_head.ck_id, "LIST")
				|| !CheckFOURCC(sdta_head.ck_id, "LIST")
				|| !CheckFOURCC(pdta_head.ck_id, "LIST")
			) {
				status = SFLIB_FAILED;
				return;
			}
			const BYTE* info_ptr = riff_buf + info_offset + sizeof(ChunkHead);
			const BYTE* sdta_ptr = riff_buf + sdta_offset + sizeof(ChunkHead);
			const BYTE* pdta_ptr = riff_buf + pdta_offset + sizeof(ChunkHead);

			infos = std::make_unique<sflib::InfoManager>(sfbk_ptr.info);
			if (infos->Status()) {
				status = infos->Status();
				return;
			}

			samples = std::make_unique<SampleManager>(sdta_ptr, sdta_head.ck_size, pdta_ptr, pdta_head.ck_size);
			if (samples->Status()) {
				status = samples->Status();
				return;
			}

			insts = std::make_unique<InstrumentManager>(sfbk_ptr.pdta);
			if (insts->Status()) {
				status = insts->Status();
				return;
			}

			presets = std::make_unique<PresetManager>(sfbk_ptr.pdta);
			if (presets->Status()) {
				status = presets->Status();
				return;
			}

			status = SFLIB_SUCCESS;
		}

		SflibError Serialize(std::ostream& os) const;

		InfoManager* Infos() { return infos.get(); }
		const InfoManager* Infos() const { return infos.get(); }
		SampleManager* Samples() { return samples.get(); }
		const SampleManager* Samples() const { return samples.get(); }
		InstrumentManager* Insts() { return insts.get(); }
		const InstrumentManager* Insts() const { return insts.get(); }
		PresetManager* Presets() { return presets.get(); }
		const PresetManager* Presets() const { return presets.get(); }

	private:
		SflibError status = SflibError::SFLIB_SUCCESS;
		std::unique_ptr<InfoManager> infos;
		std::unique_ptr<SampleManager> samples;
		std::unique_ptr<InstrumentManager> insts;
		std::unique_ptr<PresetManager> presets;
	};
}


#if 0

#ifndef SFLIB_H
#define SFLIB_H

#include <cstdint>
#include <iostream>
#include <vector>
#include <string>
#include <string_view>
#include <list>
#include <map>
#include <cstring>
#include <optional>
#include <utility>
#include <cmath>
#include <memory>

namespace sflib {
	// soundfont technical specification 2.04
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

	namespace sf2spec {
		// types/structs defined under 'sf2spec' namespace are for binary layouts of soundfont file.
	
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
	}

	template <class RecordType> 
	struct MappedChunk {
		FOURCC ck_id;
		DWORD ck_size;
		RecordType ck_data[];
	} __attribute__((packed));

	struct ChunkHead {
		FOURCC ck_id;
		DWORD ck_size;
		BYTE ck_data[];
	} __attribute__((packed));

	namespace wavspec {
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
			FOURCC data_ck_id;
			DWORD data_ck_size;
			BYTE data[];
		} __attribute__((packed));
	}

	struct SfbkMap {
		struct Info {
			const MappedChunk<sf2spec::SfVersionTag>* ifil; // the version of the Sound Font RIFF file ; e.g. 2.01
			const MappedChunk<CHAR>* isng; // target Sound Engine ; e.g. EMU8000
			const MappedChunk<CHAR>* inam;  // Sound Font Bank Name ; e.g. General MIDI
			const MappedChunk<CHAR>* irom;  // Sound ROM Name
			const MappedChunk<sf2spec::SfVersionTag>* iver; // Sound ROM Version
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
			const MappedChunk<sf2spec::SfPresetHeader>* phdr;
			const MappedChunk<sf2spec::SfPresetBag>* pbag;
			const MappedChunk<sf2spec::SfModList>* pmod;
			const MappedChunk<sf2spec::SfGenList>* pgen;
			const MappedChunk<sf2spec::SfInst>* inst;
			const MappedChunk<sf2spec::SfInstBag>* ibag;
			const MappedChunk<sf2spec::SfInstModList>* imod;
			const MappedChunk<sf2spec::SfInstGenList>* igen;
			const MappedChunk<sf2spec::SfSample>* shdr;
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

	namespace internals {

		// logs sv to stdout.
		inline void SflibLog(const std::string_view sv) {
			std::cout << "[SFLIB]" << sv;
		}

		/** @brief reads the head of sub-chunk at parent_ck_data[offset]. offset is then updated to the end of the sub-chunk.
		 * @param offset chunk offset to read (the value gets updated to the end of the sub-chunk after the call)
		 * @param ck_ptr gets set to the head of the sub-chunk (the value is undefined state when function fails)
		 * @param parent_ck_data start of the parent chunk from which you want read the sub-chunk
		 * @param parent_ck_size size of the parent chunk in bytes
		 * @return non-zero value when failed (possible causes: incomplete chunk head, violation of 16-bit alignment,
		 * sub-chunk size exceeding the parent_ck_size). zero when success.
		*/
		inline SflibError ReadChunkHead(DWORD& offset, const ChunkHead* (&ck_ptr), const BYTE* parent_ck_data, DWORD parent_ck_size) {
			if (offset + sizeof(ChunkHead) > parent_ck_size) {
				return SFLIB_FAILED;
			}
			ck_ptr = reinterpret_cast<const ChunkHead*>(parent_ck_data + offset);
			if (offset + sizeof(ChunkHead) + ck_ptr->ck_size > parent_ck_size || ck_ptr->ck_size % 2 != 0) {
				return SFLIB_FAILED;
			}
			offset += sizeof(ChunkHead) + ck_ptr->ck_size;
			return SFLIB_SUCCESS;
		}

		// maps INFO-list chunk data and its sub-chunks (data is not copied)
		inline SflibError MapInfo(SfbkMap::Info& dst, const BYTE* info_ck_data, DWORD info_ck_size) {
			DWORD fourcc;
			if (info_ck_size < sizeof(fourcc)) {
				return SFLIB_FAILED;
			}
			memcpy(&fourcc, info_ck_data, sizeof(fourcc));
			if (!CheckFOURCC(fourcc, "INFO")) {
				return SFLIB_FAILED;
			}

			DWORD offset = sizeof(fourcc);
			const ChunkHead* ck_ptr = nullptr;

			while (offset < info_ck_size) {
				auto err = ReadChunkHead(offset, ck_ptr, info_ck_data, info_ck_size);
				if (err) { return err; }

				if (CheckFOURCC(ck_ptr->ck_id, "ifil")) {
					MapChunkBinary(dst.ifil, ck_ptr);
				} else if (CheckFOURCC(ck_ptr->ck_id, "isng")) {
					MapChunkBinary(dst.isng, ck_ptr);
				} else if (CheckFOURCC(ck_ptr->ck_id, "INAM")) {
					MapChunkBinary(dst.inam, ck_ptr);
				} else if (CheckFOURCC(ck_ptr->ck_id, "irom")) {
					MapChunkBinary(dst.irom, ck_ptr);
				} else if (CheckFOURCC(ck_ptr->ck_id, "iver")) {
					MapChunkBinary(dst.iver, ck_ptr);
				} else if (CheckFOURCC(ck_ptr->ck_id, "ICRD")) {
					MapChunkBinary(dst.icrd, ck_ptr);
				} else if (CheckFOURCC(ck_ptr->ck_id, "IENG")) {
					MapChunkBinary(dst.ieng, ck_ptr);
				} else if (CheckFOURCC(ck_ptr->ck_id, "IPRD")) {
					MapChunkBinary(dst.iprd, ck_ptr);
				} else if (CheckFOURCC(ck_ptr->ck_id, "ICOP")) {
					MapChunkBinary(dst.icop, ck_ptr);
				} else if (CheckFOURCC(ck_ptr->ck_id, "ICMT")) {
					MapChunkBinary(dst.icmt, ck_ptr);
				} else if (CheckFOURCC(ck_ptr->ck_id, "ISFT")) {
					MapChunkBinary(dst.isft, ck_ptr);
				} else {
					SflibLog("Unknown chunk name: '" + FOURCCtoString(ck_ptr->ck_id) + "'. ignored.\n");
				}
			}

			return SFLIB_SUCCESS;
		}

		// maps stda chunk data and its sub-chunks (data is not copied)
		inline SflibError MapSdta(SfbkMap::Sdta& dst,const BYTE* sdta_ck_data, DWORD sdta_ck_size) {
			DWORD fourcc;
			if (sdta_ck_size < sizeof(fourcc)) {
				return SFLIB_FAILED;
			}
			memcpy(&fourcc, sdta_ck_data, sizeof(fourcc));
			if (!CheckFOURCC(fourcc, "sdta")) {
				return SFLIB_FAILED;
			}

			DWORD offset = sizeof(fourcc);
			const ChunkHead* ck_ptr = nullptr;

			while (offset < sdta_ck_size) {
				auto err = ReadChunkHead(offset, ck_ptr, sdta_ck_data, sdta_ck_size);
				if (err) { return err; }

				if (CheckFOURCC(ck_ptr->ck_id, "smpl")) {
					MapChunkBinary(dst.smpl, ck_ptr);
				} else if (CheckFOURCC(ck_ptr->ck_id, "sm24")) {
					MapChunkBinary(dst.sm24, ck_ptr);
				}
			}

			return SFLIB_SUCCESS;
		}

		// maps pdta chunk data and its sub-chunks (data is not copied)
		inline SflibError MapPdta(SfbkMap::Pdta& dst,const BYTE* pdta_ck_data, DWORD pdta_ck_size) {
			DWORD fourcc;
			if (pdta_ck_size < sizeof(fourcc)) {
				return SFLIB_FAILED;
			}
			memcpy(&fourcc, pdta_ck_data, sizeof(fourcc));
			if (!CheckFOURCC(fourcc, "pdta")) {
				return SFLIB_FAILED;
			}

			DWORD offset = sizeof(fourcc);
			const ChunkHead* ck_ptr = nullptr;

			while (offset < pdta_ck_size) {
				auto err = ReadChunkHead(offset, ck_ptr, pdta_ck_data, pdta_ck_size);
				if (err) { return err; }

				if (CheckFOURCC(ck_ptr->ck_id, "phdr")) {
					MapChunkBinary(dst.phdr, ck_ptr);
				} else if (CheckFOURCC(ck_ptr->ck_id, "pbag")) {
					MapChunkBinary(dst.pbag, ck_ptr);
				} else if (CheckFOURCC(ck_ptr->ck_id, "pmod")) {
					MapChunkBinary(dst.pmod, ck_ptr);
				} else if (CheckFOURCC(ck_ptr->ck_id, "pgen")) {
					MapChunkBinary(dst.pgen, ck_ptr);
				} else if (CheckFOURCC(ck_ptr->ck_id, "inst")) {
					MapChunkBinary(dst.inst, ck_ptr);
				} else if (CheckFOURCC(ck_ptr->ck_id, "ibag")) {
					MapChunkBinary(dst.ibag, ck_ptr);
				} else if (CheckFOURCC(ck_ptr->ck_id, "imod")) {
					MapChunkBinary(dst.imod, ck_ptr);
				} else if (CheckFOURCC(ck_ptr->ck_id, "igen")) {
					MapChunkBinary(dst.igen, ck_ptr);
				} else if (CheckFOURCC(ck_ptr->ck_id, "shdr")) {
					MapChunkBinary(dst.shdr, ck_ptr);
				}
			}

			return SFLIB_SUCCESS;
		}

		// maps all chunks from sfbk chunk
		inline SflibError GetSfbkMap(SfbkMap& dst, const BYTE* riff_ck_data, DWORD riff_ck_size) {
			memset(&dst, 0, sizeof(SfbkMap));

			DWORD fourcc;
			if (riff_ck_size < sizeof(fourcc)) {
				return SFLIB_FAILED;
			}
			memcpy(&fourcc, riff_ck_data, sizeof(fourcc));
			if (!CheckFOURCC(fourcc, "sfbk")) {
				return SFLIB_FAILED;
			}

			SflibError err;
			DWORD offset = sizeof(fourcc);
			const ChunkHead* ck_ptr = nullptr;

			err = ReadChunkHead(offset, ck_ptr, riff_ck_data, riff_ck_size);
			if (err || !CheckFOURCC(ck_ptr->ck_id, "LIST")) { return err; }

			err = MapInfo(dst.info, ck_ptr->ck_data, ck_ptr->ck_size);
			if (err) { return err; }


			err = ReadChunkHead(offset, ck_ptr, riff_ck_data, riff_ck_size);
			if (err || !CheckFOURCC(ck_ptr->ck_id, "LIST")) { return err; }

			err = MapSdta(dst.sdta, ck_ptr->ck_data, ck_ptr->ck_size);
			if (err) { return err; }


			err = ReadChunkHead(offset, ck_ptr, riff_ck_data, riff_ck_size);
			if (err || !CheckFOURCC(ck_ptr->ck_id, "LIST")) { return err; }

			err = MapPdta(dst.pdta, ck_ptr->ck_data, ck_ptr->ck_size);
			if (err) { return err; }


			return SFLIB_SUCCESS;
		}
	}

	// handles
	using SampleID = DWORD;
	using InstID = DWORD;

	template <typename T>
	struct Ranges {
		T start, end;
	};

	enum class RemovalMode {
		Restrict, Cascade, Force,
	};

	enum class SampleChannel {
		Mono, Left, Right,
	};

	class SampleManager {
	public:
		SampleManager() {} // new soundfont
		SampleManager(const SfbkMap::Sdta& sdta, const SfbkMap::Pdta& pdta) {
			const auto* shdr = pdta.shdr;
			if (!shdr) {
				state = SFLIB_FAILED;
				return;
			}
			if (shdr->ck_size % sizeof(sf2spec::SfSample) != 0) {
				state = SFLIB_INVALID_CK_SIZE;
				return;
			}

			if (sdta.smpl) {
				sample_bitdepth = 16;
				if (sdta.sm24 && ((sdta.smpl->ck_size/2 + 1) >> 1 << 1) == sdta.sm24->ck_size) {
					sample_bitdepth = 24;
				}
			}

			size_t sample_count = shdr->ck_size / sizeof(sf2spec::SfSample);
			if (sample_count == 0) {
				state = SFLIB_FAILED;
				return;
			}
			sample_count--; // ignore terminal node

			for (SampleID sample_id = 0; sample_id < sample_count; sample_id++) {
				const sf2spec::SfSample& cur_ck = shdr->ck_data[sample_id];
				
				SampleData smpl_data {};
				memcpy(smpl_data.sample_name, cur_ck.ach_sample_name, 20);
				smpl_data.sample_name[20] = 0;
				smpl_data.sample_rate = cur_ck.dw_sample_rate;
				smpl_data.start_loop = cur_ck.dw_startloop - cur_ck.dw_start;
				smpl_data.end_loop = cur_ck.dw_endloop - cur_ck.dw_start;
				smpl_data.root_key = cur_ck.by_original_key;
				smpl_data.pitch_correction = cur_ck.ch_correction;
				smpl_data.linked_sample = 0;
				smpl_data.sample_type = cur_ck.sf_sample_type;
				if (smpl_data.sample_type & (sf2spec::leftSample | sf2spec::rightSample)) {
					smpl_data.linked_sample = cur_ck.w_sample_link;
				}

				if (smpl_data.sample_type & (sf2spec::RomLeftSample & sf2spec::RomRightSample)) {
					internals::SflibLog(std::string("ROM sample detected: <") + smpl_data.sample_name + "> currently unimplemeneted.\n");
					samples[sample_id] = std::move(smpl_data);
					continue;
				}

				if (sdta.smpl && cur_ck.dw_start < cur_ck.dw_end && cur_ck.dw_end <= sdta.smpl->ck_size / 2) {
					if (sample_bitdepth == 16) {
						smpl_data.wav_data.resize((cur_ck.dw_end - cur_ck.dw_start) * 2);
						memcpy(smpl_data.wav_data.data(), &sdta.smpl->ck_data[cur_ck.dw_start * 2], smpl_data.wav_data.size());
					} else if (sample_bitdepth == 24) {
						smpl_data.wav_data.resize((cur_ck.dw_end - cur_ck.dw_start) * 3);
						for (size_t i = cur_ck.dw_start; i < cur_ck.dw_end; i++) {
							smpl_data.wav_data[3 * i + 0] = sdta.sm24->ck_data[i];
							smpl_data.wav_data[3 * i + 1] = sdta.smpl->ck_data[2 * i + 0];
							smpl_data.wav_data[3 * i + 2] = sdta.smpl->ck_data[2 * i + 1];
						}
					}
				} else {
					state = SFLIB_FAILED;
					return;
				}

				samples[sample_id] = std::move(smpl_data);
			}
			state = SFLIB_SUCCESS;
			last_smpl_id = sample_count-1;
		}

		void Serialize(int z_zone = 46) const {

		}

		SflibError State() const {
			return state;
		}

		struct WavInfo {
			wavspec::NumOfChannels num_of_channels;
			uint32_t sample_rate;
			uint16_t bit_depth;
			uint16_t block_align;
			const uint8_t* wav_data;
			uint32_t wav_size;
		};

		SflibResult<SampleID> AddMono(const void* wav_data, size_t wav_size,
			std::optional<const std::string&> name = std::nullopt,
			std::optional<Ranges<uint32_t>> loop = std::nullopt,
			std::optional<uint16_t> root_key = std::nullopt,
			std::optional<int16_t> pitch_correction = std::nullopt,
			SampleChannel sample_type = SampleChannel::Mono
		) {
			auto [wav_info, err] = SampleManager::ValidateWav(wav_data, wav_size);
			if (err) {
				return { 0, err };
			}
			if (wav_info.bit_depth != this->sample_bitdepth) {
				return { 0, SFLIB_INCOMPATIBLE_BIT_DEPTH };
			}
			if (sample_type == SampleChannel::Mono) {
				if (wav_info.num_of_channels != wavspec::ChannelMono) {
					return { 0, SFLIB_NOT_MONO_CHANNEL };
				}
			} else {
				if (wav_info.num_of_channels == wavspec::ChannelMono) {
					return { 0, SFLIB_NOT_STEREO_CHANNEL };
				}
			}
			
			SampleData sdata;
			if (name) {
				memcpy(sdata.sample_name, (*name).c_str(), std::min<size_t>(20, (*name).length()));
				sdata.sample_name[20] = 0;
			}
			if (loop) {
				sdata.start_loop = (*loop).start;
				sdata.end_loop = (*loop).end;
			}
			if (root_key) {
				sdata.root_key = *root_key;
			}
			if (pitch_correction) {
				sdata.pitch_correction = *pitch_correction;
			}

			sdata.sample_rate = wav_info.sample_rate;
			if (sample_type == SampleChannel::Mono) {
				sdata.wav_data.resize(wav_info.wav_size);
				memcpy(sdata.wav_data.data(), wav_info.wav_data, wav_info.wav_size);
			} else {
				const size_t data_size = wav_info.wav_size / 2;
				const size_t bytes_per_sample = wav_info.bit_depth / 8;

				sdata.wav_data.resize(data_size);
				for (size_t i = 0, j = 0; i < data_size; i += bytes_per_sample, j++) { // left sample
					size_t offset = (sample_type == SampleChannel::Left) ? 0 : bytes_per_sample;
					memcpy(&sdata.wav_data[i],
						&wav_info.wav_data[j * wav_info.block_align + offset], bytes_per_sample);
				}
			}
			sdata.sample_type = sf2spec::monoSample;
			sdata.linked_sample = 0;
			this->samples.emplace(++(this->last_smpl_id), sdata);
			return { this->last_smpl_id, SFLIB_SUCCESS };
		}

		SflibResult<SampleID> AddStereo(const void* wav_data, size_t wav_size,
			std::optional<const std::string&> name = std::nullopt,
			std::optional<Ranges<uint32_t>> loop = std::nullopt,
			std::optional<uint16_t> root_key = std::nullopt,
			std::optional<int16_t> pitch_correction = std::nullopt
		) {
			std::string base_name = name.has_value() ? (*name).substr(0, 18) : "DUMMY";
			auto [ left_id, err1] = AddMono(wav_data, wav_size, base_name + "_L", loop, root_key, pitch_correction, SampleChannel::Left);
			auto [right_id, err2] = AddMono(wav_data, wav_size, base_name + "_R", loop, root_key, pitch_correction, SampleChannel::Right);
			if (err1 | err2) {
				SflibError err;
				if (err1 == SFLIB_SUCCESS) {
					err = err2;
					Remove( left_id, RemovalMode::Force);
				}
				if (err2 == SFLIB_SUCCESS) {
					err = err1;
					Remove(right_id, RemovalMode::Force);
				}
				return { 0, err };
			}

			return { left_id, LinkStereo(left_id, right_id) };
		}

		SflibError Remove(SampleID id, RemovalMode mode=RemovalMode::Restrict) {
			auto it = samples.find(id);
			if (it == samples.end()) {
				return SFLIB_FAILED;
			}
			std::optional<decltype(it)> it2_opt;
			if (it->second.sample_type & (sf2spec::leftSample | sf2spec::rightSample)) {
				it2_opt = samples.find(it->second.linked_sample);
				if (*it2_opt == samples.end()) {
					return SFLIB_FAILED;
				}
			}

			if (!it2_opt.has_value()) {
				auto ref = referenced.find(id);
				switch (mode) {
				case RemovalMode::Restrict:
				case RemovalMode::Cascade:
					if (ref == referenced.end()) {
						samples.erase(it);
						return SFLIB_SUCCESS;
					} else {
						return SFLIB_FAILED;
					}
				case RemovalMode::Force:
					samples.erase(it);
					return SFLIB_SUCCESS;
				}
			} else {
				auto ref1 = referenced.find(id);
				auto ref2 = referenced.find(it2_opt.value()->first);
				switch (mode) {
				case RemovalMode::Restrict:
				case RemovalMode::Cascade:
					if (ref1 == referenced.end() && ref2 == referenced.end()) {
						samples.erase(it);
						return SFLIB_SUCCESS;
					} else {
						return SFLIB_FAILED;
					}
				case RemovalMode::Force:
					samples.erase(it);
					return SFLIB_SUCCESS;	
				}
			}
		}

		SflibError LinkStereo(SampleID left, SampleID right) {
			auto it_left = samples.find(left);
			auto it_right = samples.find(right);
			if (it_left == samples.end() || it_right == samples.end()) {
				return SFLIB_NO_SUCH_SAMPLE;
			}

			SampleData& smpl_left = it_left->second;
			SampleData& smpl_right = it_right->second;
			if (smpl_left.wav_data.size() != smpl_right.wav_data.size()
				|| smpl_left.sample_rate != smpl_right.sample_rate
				|| smpl_left.start_loop != smpl_right.start_loop
				|| smpl_right.end_loop != smpl_right.end_loop
			) {
				return SFLIB_BAD_LINK;
			}

			it_left->second.sample_type = sf2spec::leftSample;
			it_right->second.sample_type = sf2spec::rightSample;
			it_left->second.linked_sample = right;
			it_right->second.linked_sample = left;
			
			return SFLIB_SUCCESS;
		}

		void SetName(SampleID id, const std::string& value) {
			memcpy(samples.at(id).sample_name, value.c_str(), 20);
		}
		std::string GetName(SampleID id) const {
			return samples.at(id).sample_name;
		}
		void SetLoopPoint(SampleID id, uint32_t start, uint32_t end) {
			auto& smpl = samples.at(id);
			smpl.start_loop = start;
			smpl.end_loop = end;
		}
		Ranges<uint32_t> GetLoopPoint(SampleID id) const {
			const auto& smpl = samples.at(id);
			return { smpl.start_loop, smpl.end_loop };
		}
		void SetRootKey(SampleID id, uint16_t value) {
			samples.at(id).root_key = value;
		}
		uint16_t GetRootKey(SampleID id) const {
			return samples.at(id).root_key;
		}
		void SetPitchCorrection(SampleID id, int16_t value) {
			samples.at(id).pitch_correction = value;
		}
		int16_t GetPitchCorrection(SampleID id) const {
			return samples.at(id).pitch_correction;
		}

		int32_t GetSampleRate(SampleID id) const {
			return samples.at(id).sample_rate;
		}
		SampleID GetLinkedSampleID(SampleID id) const {
			return samples.at(id).linked_sample;
		}
		bool IsMono(SampleID id) const {
			return (samples.at(id).sample_type & sf2spec::monoSample) != 0;
		}
		bool IsLeftChannel(SampleID id) const {
			return (samples.at(id).sample_type & sf2spec::leftSample) != 0;
		}
		bool IsRightChannel(SampleID id) const {
			return (samples.at(id).sample_type & sf2spec::rightSample) != 0;
		}
		bool IsRomSample(SampleID id) const {
			return (samples.at(id).sample_type & (sf2spec::RomLeftSample & sf2spec::RomRightSample)) != 0;
		}

		static SflibResult<WavInfo> ValidateWav(const void* data, size_t size) {
			if (size < 44) {
				return { {}, SFLIB_BAD_WAV_DATA };
			}
			const auto* riff = reinterpret_cast<const ChunkHead*>(data);
			if (!(internals::CheckFOURCC(riff->ck_id, "RIFF") && riff->ck_size >= 36 && riff->ck_size <= size - 8)) {
				return { {}, SFLIB_BAD_WAV_DATA };
			}
			FOURCC fmt;
			memcpy(&fmt, riff->ck_data, sizeof(FOURCC));
			if (!internals::CheckFOURCC(fmt, "WAVE")) {
				return { {}, SFLIB_BAD_WAV_DATA };
			}
			const auto* header = reinterpret_cast<const wavspec::WaveFmtChunk*>(riff->ck_data + sizeof(FOURCC));
			if (!(internals::CheckFOURCC(header->ck_id, "fmt ") && header->ck_size == 16)) {
				return { {}, SFLIB_BAD_WAV_DATA };
			}
			unsigned ch_nums = static_cast<unsigned>(header->num_of_channels);
			if (header->num_of_channels == wavspec::Channel4_Type2) {
				ch_nums = 4;
			}
			if (
				header->byte_rate != (header->bits_per_sample / 8) * header->sample_rate * ch_nums
				|| header->block_align != (header->bits_per_sample / 8) * ch_nums
				|| !internals::CheckFOURCC(header->data_ck_id, "data")
				|| header->data_ck_size > riff->ck_size - 36
			) {
				return { {}, SFLIB_BAD_WAV_DATA };
			}

			SflibError err = SFLIB_SUCCESS;
			if (!(
				header->audio_format == wavspec::AudioFormatPCM
				&& (header->num_of_channels == wavspec::ChannelMono || header->num_of_channels == wavspec::ChannelStereo)
				&& (header->bits_per_sample == 16 || header->bits_per_sample == 24)
			)) {
				err = SFLIB_UNSUPPORTED_WAV_DATA;
			}

			//if (header->bits_per_sample != this->sample_bitdepth) {
			//	return SFLIB_INCOMPATIBLE_BIT_DEPTH;
			//}

			return {
				WavInfo {
					.num_of_channels = header->num_of_channels,
					.sample_rate = header->sample_rate,
					.bit_depth = header->bits_per_sample,
					.block_align = header->block_align,
					.wav_data = header->data,
					.wav_size = header->data_ck_size
				}, err
			};
		}

		SflibError AddRef(SampleID sample, InstID inst) {
			if (auto it = samples.find(sample); it == samples.end()) {
				return SFLIB_NO_SUCH_SAMPLE;
			}
			referenced.emplace(sample, inst);
		}
		SflibError RemoveRef(SampleID sample, InstID inst) {
			auto [first, last] = referenced.equal_range(sample);

			auto it = std::find_if(first, last, [inst](auto&& pair) { return pair.second == inst; });
			if (it == last) {
				return SFLIB_FAILED;
			}
			referenced.erase(it);
			return SFLIB_SUCCESS;
		}

	private:
		struct SampleData {
			char sample_name[21] {};
			std::vector<BYTE> wav_data {};
			DWORD sample_rate = 0;
			DWORD start_loop = 0;
			DWORD end_loop = 0;
			WORD root_key = 0;
			SHORT pitch_correction = 0;
			SampleID linked_sample = 0;
			sf2spec::SFSampleLink sample_type = sf2spec::monoSample;

			SampleData() {}
			SampleData(const SampleData&) = delete;
			SampleData& operator=(SampleData&) = delete;
			SampleData(SampleData&&) = default;
			SampleData& operator=(SampleData&&) = default;
			void CopyParameters(const SampleData& rhs) {
				memcpy(sample_name, rhs.sample_name, 21);
				sample_rate = rhs.sample_rate;
				start_loop = rhs.start_loop;
				end_loop = rhs.end_loop;
				root_key = root_key;
				pitch_correction = pitch_correction;
			}
		};

		SflibError state;
		SampleID last_smpl_id = 0;
		int sample_bitdepth = 16;
		std::map<SampleID, SampleData> samples;
		std::multimap<SampleID, InstID> referenced;
	};


	class SoundFont2 {
	public:
		SoundFont2(std::istream& is) {
			// TODO: improve error handling...
			ChunkHead ck_head {};
			is.read((char*)&ck_head, sizeof(ChunkHead));

			if (!internals::CheckFOURCC(ck_head.ck_id, "RIFF")) {
				throw "Illegal format.";
			}

			std::vector<BYTE> buf(ck_head.ck_size);
			is.read((char*)buf.data(), ck_head.ck_size);

			SfbkMap sfbk_map;
			if (auto err = internals::GetSfbkMap(sfbk_map, buf.data(), buf.size())) {
				throw "Illegal format.";
			}

			if (auto err = InitInfo(sfbk_map.info)) {
				throw "Illegal format.";
			}
			sample_manager = std::make_unique<SampleManager>(sfbk_map.sdta, sfbk_map.pdta);
			if (sample_manager->State() != SFLIB_SUCCESS) {
				throw "Illegal format.";
			}
			if (auto err = InitInstruments(sfbk_map.pdta)) {
				throw "Illegal format.";
			}
			if (auto err = InitPresets(sfbk_map.pdta)) {
				throw "Illegal format.";
			}
		}


		void PrintInfo() const {
			auto version_tag_str = [](const InfoData::VersionTag& vtag) {
				std::string buf("x.xx");
				sprintf(buf.data(), "%.1d.%02d", vtag.major, vtag.minor % 100);
				return buf;
			};
			std::cout << "SountFont Version: " << version_tag_str(info_data.sf_version) << "\n";
			std::cout << "Target Sound Engine: " << info_data.target_sound_engine << "\n";
			std::cout << "SountFont Bank Name: " << info_data.sf_bank_name << "\n";
			if (info_data.sound_rom_name && info_data.sound_rom_version) {
				std::cout << "Sound ROM(ver.): " << *info_data.sound_rom_name
				          << " (v" << version_tag_str(*info_data.sound_rom_version) << ")\n";
			}
			if (info_data.creation_date) {
				std::cout << "Creation Date: " << *info_data.creation_date << "\n";
			}
			if (info_data.author) {
				std::cout << "Author(s): " << *info_data.author << "\n";
			}
			if (info_data.target_product) {
				std::cout << "Target Product: " << *info_data.target_product << "\n";
			}
			if (info_data.copyright_msg) {
				std::cout << "Copyright: " << *info_data.copyright_msg << "\n";
			}
			if (info_data.comments) {
				std::cout << "Comments: " << *info_data.comments << "\n";
			}
			if (info_data.sf_tools) {
				std::cout << "Used Tools: " << *info_data.sf_tools << "\n";
			}
		}


		void PrintPresets() const {
			char buf[] = "[xxx:xxx]";
			for (const auto& preset : presets) {
				if (preset.empty()) {
					continue;
				}
				for (const auto& bnk : preset) {
					sprintf(buf, "%03d:%03d", bnk.bank_number % 1000, bnk.preset_number % 1000);
					std::cout << bnk.preset_name << " " << buf << "\n";
					for (const auto& zone : bnk.zones) {
						if (zone.is_global) {
							std::cout << "  (global)\n";
						} else {
							auto it = instruments.find(zone.inst_id);
							std::cout << "  <" << it->second.inst_name << ">\n";
						}
					}
				}
			}
		}

		SampleManager* Samples() {
			return sample_manager.get();
		}

	private:
		std::unique_ptr<SampleManager> sample_manager;

		struct InfoData {
			struct VersionTag { unsigned major = 0, minor = 0; };

			VersionTag sf_version;
			std::string target_sound_engine;
			std::string sf_bank_name;

			std::optional<std::string> sound_rom_name;
			std::optional<VersionTag> sound_rom_version;
			std::optional<std::string> creation_date;
			std::optional<std::string> author;
			std::optional<std::string> target_product;
			std::optional<std::string> copyright_msg;
			std::optional<std::string> comments;
			std::optional<std::string> sf_tools;
		} info_data;

		struct InstData {
			char inst_name[21];
			struct InstZone {
				bool is_global = false;
				SampleID sample_id;
				std::map<SFGenerator, sf2spec::GenAmountType> others;
			};
			std::vector<InstZone> zones;
		};

		struct PresetData {
			WORD bank_number;
			WORD preset_number;
			char preset_name[21];
			struct PresetZone {
				bool is_global = false;
				InstID inst_id;
				std::map<SFGenerator, sf2spec::GenAmountType> others;
			};
			std::vector<PresetZone> zones;
		};

		std::map<InstID, InstData> instruments;
		std::vector<std::vector<PresetData>> presets;

		SflibError InitInfo(const SfbkMap::Info& src) {
			// validates whether the string is zero-terminated
			auto validate_zstr = [](const auto* ck) -> bool {
				return ck->ck_data[ck->ck_size - 1] == 0x00;
			};

			if (!src.ifil || !src.isng || !src.inam) {
				return SFLIB_FAILED;
			}

			/* reading mandatory INFO sub chunks */
			if (src.ifil->ck_size == 4) {
				info_data.sf_version.major = src.ifil->ck_data[0].w_major;
				info_data.sf_version.minor = src.ifil->ck_data[0].w_minor;
			} else {
				return SFLIB_INVALID_CK_SIZE;
			}
			
			if (validate_zstr(src.isng)) {
				info_data.target_sound_engine.assign(reinterpret_cast<const char*>(src.isng->ck_data));
			} else {
				info_data.target_sound_engine.assign("EMU8000");
			}

			if (validate_zstr(src.inam)) {
				info_data.sf_bank_name.assign(reinterpret_cast<const char*>(src.inam->ck_data));
			} else {
				// the sfspec24 specifies that the program should prompt the user to fix the bank name,
				// but i'm too lazy to implement that
				return SFLIB_ZSTR_CHECK_FAILED;
			}


			/* reading optional INFO sub chunks */
			if (src.irom && validate_zstr(src.irom)) {
				info_data.sound_rom_name = std::string(reinterpret_cast<const char*>(src.irom->ck_data));
			}
			if (src.iver && src.iver->ck_size == 4) {
				info_data.sound_rom_version = InfoData::VersionTag {
					.major = src.iver->ck_data[0].w_major,
					.minor = src.iver->ck_data[0].w_minor
				};
			}
			if (src.icrd && validate_zstr(src.icrd)) {
				info_data.creation_date = std::string(reinterpret_cast<const char*>(src.icrd->ck_data));
			}
			if (src.ieng && validate_zstr(src.ieng)) {
				info_data.author = std::string(reinterpret_cast<const char*>(src.ieng->ck_data));
			}
			if (src.iprd && validate_zstr(src.iprd)) {
				info_data.target_product = std::string(reinterpret_cast<const char*>(src.iprd->ck_data));
			}
			if (src.icop && validate_zstr(src.icop)) {
				info_data.copyright_msg = std::string(reinterpret_cast<const char*>(src.icop->ck_data));
			}
			if (src.icmt && validate_zstr(src.icmt)) {
				info_data.comments = std::string(reinterpret_cast<const char*>(src.icmt->ck_data));
			}
			if (src.isft && validate_zstr(src.isft)) {
				info_data.sf_tools = std::string(reinterpret_cast<const char*>(src.isft->ck_data));
			}
			return SFLIB_SUCCESS;
		}
	
		SflibError InitInstruments(const SfbkMap::Pdta& pdta) {
			if (!pdta.inst || !pdta.ibag || !pdta.imod || !pdta.igen) {
				return SFLIB_FAILED;
			}

			// validate inst, ibag, imod, igen chunks here! v v v

			// validate inst, ibag, imod, igen chunks here! ^ ^ ^

			size_t inst_count = pdta.inst->ck_size / sizeof(sf2spec::SfInst) - 1;
			for (InstID id = 0; id < inst_count; id++) {
				const sf2spec::SfInst* cur = &pdta.inst->ck_data[id];

				InstData res {};
				memcpy(res.inst_name, cur->ach_inst_name, 20);
				res.inst_name[20] = 0;

				const size_t bag_start = cur->w_inst_bag_ndx;
				const size_t bag_end = (cur + 1)->w_inst_bag_ndx;
				for (size_t bag_ndx = bag_start; bag_ndx < bag_end; bag_ndx++) {
					const sf2spec::SfInstBag* bag = &pdta.ibag->ck_data[bag_ndx];
					// IMOD is skipped rn...
					InstData::InstZone zone;
					zone.is_global = true;
					zone.sample_id = 0;
					const size_t gen_start = bag->w_inst_gen_ndx;
					const size_t gen_end = (bag + 1)->w_inst_gen_ndx;
					for (size_t gen_ndx = gen_start; gen_ndx < gen_end; gen_ndx++) {
						const sf2spec::SfInstGenList& gen = pdta.igen->ck_data[gen_ndx];
						if (gen.sf_gen_oper == SfSampleID) { // terminal generator; generators that appear after this should be ignored.
							zone.is_global = false;
							zone.sample_id = gen.gen_amount.w_amount;
							break;
						} else {
							// if the same operator is encountered more than twice, only the last one will be valid.
							zone.others[gen.sf_gen_oper] = gen.gen_amount;
						}
					}
					res.zones.push_back(std::move(zone));
				}
				instruments[id] = std::move(res);
			}
			return SFLIB_SUCCESS;
		}

		SflibError InitPresets(const SfbkMap::Pdta& pdta) {
			if (!pdta.phdr || !pdta.pbag || !pdta.pmod || !pdta.pgen) {
				return SFLIB_FAILED;
			}

			// validate phdr, pbag, pmod, pgen chunks here! v v v

			// validate phdr, pbag, pmod, pgen chunks here! ^ ^ ^

			presets.clear();
			presets.resize(128); // possible preset numbers: 0 ~ 127

			size_t preset_count = pdta.phdr->ck_size / sizeof(sf2spec::SfPresetHeader) - 1;
			for (DWORD i = 0; i < preset_count; i++) {
				const sf2spec::SfPresetHeader* cur = &pdta.phdr->ck_data[i];

				PresetData res {};
				memcpy(res.preset_name, cur->ach_preset_name, 20);
				res.preset_name[20] = 0;
				res.preset_number = cur->w_preset;
				res.bank_number = cur->w_bank;

				const size_t bag_start = cur->w_preset_bag_ndx;
				const size_t bag_end = (cur + 1)->w_preset_bag_ndx;
				for (size_t bag_ndx = bag_start; bag_ndx < bag_end; bag_ndx++) {
					const sf2spec::SfPresetBag* bag = &pdta.pbag->ck_data[bag_ndx];
					// IMOD is skipped rn...
					PresetData::PresetZone zone;
					zone.is_global = true;
					zone.inst_id = 0;
					const size_t gen_start = bag->w_gen_ndx;
					const size_t gen_end = (bag + 1)->w_gen_ndx;
					for (size_t gen_ndx = gen_start; gen_ndx < gen_end; gen_ndx++) {
						const sf2spec::SfGenList& gen = pdta.pgen->ck_data[gen_ndx];
						if (gen.sf_gen_oper == SfInstrument) { // terminal generator; generators that appear after this should be ignored.
							zone.is_global = false;
							zone.inst_id = gen.gen_amount.w_amount;
							break;
						} else {
							// if the same operator is encountered more than twice, only the last one will be valid.
							zone.others[gen.sf_gen_oper] = gen.gen_amount;
						}
					}
					res.zones.push_back(std::move(zone));
				}
				presets[res.preset_number].push_back(std::move(res));
			}
			return SFLIB_SUCCESS;
		}
	};
}

#endif // SFLIB_H

#endif