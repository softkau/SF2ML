#include "sfspec.hpp"
#include "info_manager.hpp"

#include <cstddef>
#include <memory>
#include <cstring>

using namespace sflib;

class SampleManager {

};

class SoundFontManager {
public:
	SoundFontManager(const BYTE* buf, size_t buf_size);

	SflibError Status() const { return status; }
private:
	
	std::unique_ptr<InfoManager> info_manager;
	std::unique_ptr<SampleManager> sample_manager;


	SflibError status = SflibError::SFLIB_SUCCESS;
};

SoundFontManager::SoundFontManager(const BYTE* buf, std::size_t buf_size) {
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

	ChunkHead info_head;
	auto [sdta_offset, info_err] = ReadChunkHead(info_head, riff_buf, riff_size, 0);
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
	
}