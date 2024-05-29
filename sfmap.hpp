#pragma once

#include "sfspec.hpp"

namespace sflib {
	struct SfbkMap {
		struct Info {
			const BYTE* ifil; // the version of the Sound Font RIFF file ; e.g. 2.01
			const BYTE* isng; // target Sound Engine ; e.g. EMU8000
			const BYTE* inam; // Sound Font Bank Name ; e.g. General MIDI
			const BYTE* irom; // Sound ROM Name
			const BYTE* iver; // Sound ROM Version
			const BYTE* icrd; // date of creation of the bank
			const BYTE* ieng; // sound designers and engineers for the bank
			const BYTE* iprd; // product for which the bank was intended
			const BYTE* icop; // contains any copyright message
			const BYTE* icmt; // contains any comments on the bank
			const BYTE* isft; // the SoundFont tools used to create and alter the bank
		} info;
		struct Sdta {
			const BYTE* smpl; // 16-bit samples (when sm24 chunk exists, it is interpreted as upper 16-bit part)
			const BYTE* sm24; // lower 8-bit for 24-bit samples
		} sdta;
		struct Pdta {
			const BYTE* phdr;
			const BYTE* pbag;
			const BYTE* pmod;
			const BYTE* pgen;
			const BYTE* inst;
			const BYTE* ibag;
			const BYTE* imod;
			const BYTE* igen;
			const BYTE* shdr;
		} pdta;
	};

	SflibError GetSfbkMap(SfbkMap& dst, const BYTE* riff_ck_data, DWORD riff_ck_size);
}