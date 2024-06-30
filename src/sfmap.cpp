#include "sfmap.hpp"

namespace SF2ML {
	/** @brief reads the head of sub-chunk at parent_ck_data[offset]. offset is then updated to the end of the sub-chunk.
	 * @param offset chunk offset to read (the value gets updated to the end of the sub-chunk after the call)
	 * @param ck gets set to the head of the sub-chunk (the value is undefined state when function fails)
	 * @param parent_ck_data start of the parent chunk from which you want read the sub-chunk
	 * @param parent_ck_size size of the parent chunk in bytes
	 * @return non-zero value when failed (possible causes: incomplete chunk head, violation of 16-bit alignment,
	 * sub-chunk size exceeding the parent_ck_size). zero when success.
	*/
	SF2MLError ReadChunkHead(DWORD& offset, ChunkHead* ck_ptr, const BYTE* parent_ck_data, DWORD parent_ck_size) {
		if (offset + sizeof(ChunkHead) > parent_ck_size) {
			return SF2ML_FAILED;
		}
		std::memcpy(ck_ptr, parent_ck_data + offset, sizeof(ChunkHead));
		if (offset + sizeof(ChunkHead) + ck_ptr->ck_size > parent_ck_size || ck_ptr->ck_size % 2 != 0) {
			return SF2ML_FAILED;
		}
		offset += sizeof(ChunkHead) + ck_ptr->ck_size;
		return SF2ML_SUCCESS;
	}

	SF2MLError MapInfo(SfbkMap::Info& dst, const BYTE* info_ck_data, DWORD info_ck_size) {
		DWORD fourcc;
		if (info_ck_size < sizeof(fourcc)) {
			return SF2ML_FAILED;
		}
		memcpy(&fourcc, info_ck_data, sizeof(fourcc));
		if (!CheckFOURCC(fourcc, "INFO")) {
			return SF2ML_FAILED;
		}

		DWORD offset = sizeof(fourcc);
		ChunkHead ck;

		while (offset < info_ck_size) {
			const BYTE* ptr = info_ck_data + offset;
			auto err = ReadChunkHead(offset, &ck, info_ck_data, info_ck_size);
			if (err) { return err; }

			if (CheckFOURCC(ck.ck_id, "ifil")) {
				dst.ifil = ptr;
			} else if (CheckFOURCC(ck.ck_id, "isng")) {
				dst.isng = ptr;
			} else if (CheckFOURCC(ck.ck_id, "INAM")) {
				dst.inam = ptr;
			} else if (CheckFOURCC(ck.ck_id, "irom")) {
				dst.irom = ptr;
			} else if (CheckFOURCC(ck.ck_id, "iver")) {
				dst.iver = ptr;
			} else if (CheckFOURCC(ck.ck_id, "ICRD")) {
				dst.icrd = ptr;
			} else if (CheckFOURCC(ck.ck_id, "IENG")) {
				dst.ieng = ptr;
			} else if (CheckFOURCC(ck.ck_id, "IPRD")) {
				dst.iprd = ptr;
			} else if (CheckFOURCC(ck.ck_id, "ICOP")) {
				dst.icop = ptr;
			} else if (CheckFOURCC(ck.ck_id, "ICMT")) {
				dst.icmt = ptr;
			} else if (CheckFOURCC(ck.ck_id, "ISFT")) {
				dst.isft = ptr;
			}
		}

		return SF2ML_SUCCESS;
	}

	// maps stda chunk data and its sub-chunks (data is not copied)
	SF2MLError MapSdta(SfbkMap::Sdta& dst,const BYTE* sdta_ck_data, DWORD sdta_ck_size) {
		DWORD fourcc;
		if (sdta_ck_size < sizeof(fourcc)) {
			return SF2ML_FAILED;
		}
		memcpy(&fourcc, sdta_ck_data, sizeof(fourcc));
		if (!CheckFOURCC(fourcc, "sdta")) {
			return SF2ML_FAILED;
		}

		DWORD offset = sizeof(fourcc);
		ChunkHead ck;

		while (offset < sdta_ck_size) {
			const BYTE* ptr = sdta_ck_data + offset;
			auto err = ReadChunkHead(offset, &ck, sdta_ck_data, sdta_ck_size);
			if (err) { return err; }

			if (CheckFOURCC(ck.ck_id, "smpl")) {
				dst.smpl = ptr;
			} else if (CheckFOURCC(ck.ck_id, "sm24")) {
				dst.sm24 = ptr;
			}
		}

		return SF2ML_SUCCESS;
	}

	// maps pdta chunk data and its sub-chunks (data is not copied)
	SF2MLError MapPdta(SfbkMap::Pdta& dst, const BYTE* pdta_ck_data, DWORD pdta_ck_size) {
		DWORD fourcc;
		if (pdta_ck_size < sizeof(fourcc)) {
			return SF2ML_FAILED;
		}
		memcpy(&fourcc, pdta_ck_data, sizeof(fourcc));
		if (!CheckFOURCC(fourcc, "pdta")) {
			return SF2ML_FAILED;
		}

		DWORD offset = sizeof(fourcc);
		ChunkHead ck;

		while (offset < pdta_ck_size) {
			const BYTE* ptr = pdta_ck_data + offset;
			auto err = ReadChunkHead(offset, &ck, pdta_ck_data, pdta_ck_size);
			if (err) { return err; }

			if (CheckFOURCC(ck.ck_id, "phdr")) {
				dst.phdr = ptr;
			} else if (CheckFOURCC(ck.ck_id, "pbag")) {
				dst.pbag = ptr;
			} else if (CheckFOURCC(ck.ck_id, "pmod")) {
				dst.pmod = ptr;
			} else if (CheckFOURCC(ck.ck_id, "pgen")) {
				dst.pgen = ptr;
			} else if (CheckFOURCC(ck.ck_id, "inst")) {
				dst.inst = ptr;
			} else if (CheckFOURCC(ck.ck_id, "ibag")) {
				dst.ibag = ptr;
			} else if (CheckFOURCC(ck.ck_id, "imod")) {
				dst.imod = ptr;
			} else if (CheckFOURCC(ck.ck_id, "igen")) {
				dst.igen = ptr;
			} else if (CheckFOURCC(ck.ck_id, "shdr")) {
				dst.shdr = ptr;
			}
		}

		return SF2ML_SUCCESS;
	}

	// maps all chunks from sfbk chunk
	SF2MLError GetSfbkMap(SfbkMap& dst, const BYTE* riff_ck_data, DWORD riff_ck_size) {
		memset(&dst, 0, sizeof(SfbkMap));

		DWORD fourcc;
		if (riff_ck_size < sizeof(fourcc)) {
			return SF2ML_FAILED;
		}
		memcpy(&fourcc, riff_ck_data, sizeof(fourcc));
		if (!CheckFOURCC(fourcc, "sfbk")) {
			return SF2ML_FAILED;
		}

		SF2MLError err;
		DWORD offset = sizeof(fourcc);
		ChunkHead ck;

		DWORD info_off = offset;
		err = ReadChunkHead(offset, &ck, riff_ck_data, riff_ck_size);
		if (err || !CheckFOURCC(ck.ck_id, "LIST")) { return err; }

		err = MapInfo(dst.info, riff_ck_data + info_off + 8, ck.ck_size);
		if (err) { return err; }

		DWORD sdta_off = offset;
		err = ReadChunkHead(offset, &ck, riff_ck_data, riff_ck_size);
		if (err || !CheckFOURCC(ck.ck_id, "LIST")) { return err; }

		err = MapSdta(dst.sdta, riff_ck_data + sdta_off + 8, ck.ck_size);
		if (err) { return err; }

		DWORD pdta_off = offset;
		err = ReadChunkHead(offset, &ck, riff_ck_data, riff_ck_size);
		if (err || !CheckFOURCC(ck.ck_id, "LIST")) { return err; }

		err = MapPdta(dst.pdta, riff_ck_data + pdta_off + 8, ck.ck_size);
		if (err) { return err; }

		return SF2ML_SUCCESS;
	}
}