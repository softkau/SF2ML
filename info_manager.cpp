#include "info_manager.hpp"
#include <cstring>
#include <cstddef>
#include <iostream>

using namespace sflib;

static void DefaultLogger(const std::string& msg) {
	std::cout << "[SFLIB]" << msg << "\n";
}

InfoManager::InfoManager(const BYTE* buf, std::size_t buf_size) : logger(DefaultLogger) {
	DWORD fourcc;
	if (buf_size < sizeof(fourcc));
	std::memcpy(&fourcc, buf, sizeof(fourcc));
	if (!CheckFOURCC(fourcc, "INFO")) {
		status = SFLIB_FAILED;
		return;
	}

	bool ifil_exists = false;
	bool isng_exists = false;
	bool inam_exists = false;

	DWORD offset = sizeof(fourcc);
	ChunkHead ck_head;
	while (offset < buf_size) {
		auto [next, err] = ReadChunkHead(ck_head, buf, buf_size, offset);
		if (err) {
			status = err;
			return;
		}
		DWORD sub_offset = offset + sizeof(ChunkHead);
		auto validate_zstr = [buf, sub_offset, ck_head]() -> bool {
			return buf[sub_offset + ck_head.ck_size - 1] == 0x00;
		};
		offset = next;

		if (CheckFOURCC(ck_head.ck_id, "ifil")) {
			if (ck_head.ck_size != 4) {
				status = SFLIB_INVALID_CK_SIZE;
				return;
			}
			SfVersionTag ver;
			std::memcpy(&ver, buf + sub_offset, sizeof(ver));
			sf_version.major = ver.w_major;
			sf_version.minor = ver.w_minor;
			ifil_exists = true;
		} else if (CheckFOURCC(ck_head.ck_id, "isng")) {
			if (validate_zstr()) {
				target_sound_engine.assign(reinterpret_cast<const char*>(buf + sub_offset));
			} else {
				target_sound_engine.assign("EMU8000");
			}
			isng_exists = true;
		} else if (CheckFOURCC(ck_head.ck_id, "INAM")) {
			if (validate_zstr()) {
				sf_bank_name.assign(reinterpret_cast<const char*>(buf + sub_offset));
			} else {
				status = SFLIB_ZSTR_CHECK_FAILED;
				return;
			}
			inam_exists = true;
		} else if (CheckFOURCC(ck_head.ck_id, "irom") && validate_zstr()) {
			sound_rom_name = std::string(reinterpret_cast<const char*>(buf + sub_offset));
		} else if (CheckFOURCC(ck_head.ck_id, "iver") && ck_head.ck_size == 4) {
			SfVersionTag ver;
			std::memcpy(&ver, buf + sub_offset, sizeof(ver));
			sound_rom_version = VersionTag {
				.major = ver.w_major,
				.minor = ver.w_minor
			};
		}
		else if (CheckFOURCC(ck_head.ck_id, "ICRD") && validate_zstr()) {
			creation_date = std::string(reinterpret_cast<const char*>(buf + sub_offset));
		} else if (CheckFOURCC(ck_head.ck_id, "IENG")) {
			author = std::string(reinterpret_cast<const char*>(buf + sub_offset));
		} else if (CheckFOURCC(ck_head.ck_id, "IPRD")) {
			target_product = std::string(reinterpret_cast<const char*>(buf + sub_offset));
		} else if (CheckFOURCC(ck_head.ck_id, "ICOP")) {
			copyright_msg = std::string(reinterpret_cast<const char*>(buf + sub_offset));
		} else if (CheckFOURCC(ck_head.ck_id, "ICMT")) {
			comments = std::string(reinterpret_cast<const char*>(buf + sub_offset));
		} else if (CheckFOURCC(ck_head.ck_id, "ISFT")) {
			sf_tools = std::string(reinterpret_cast<const char*>(buf + sub_offset));
		} else {
			logger("unknown chunk ID: '" + FOURCCtoString(ck_head.ck_id) + "'. ignored.");
		}
	}

	if (!ifil_exists || !isng_exists || !inam_exists) {
		status = SFLIB_FAILED;
		return;
	}
}

void InfoManager::PrintInfo(std::ostream& os) const {
	auto version_tag_str = [](const VersionTag& vtag) {
		std::string buf("x.xx");
		std::sprintf(buf.data(), "%.1d.%02d", vtag.major, vtag.minor % 100);
		return buf;
	};
	os << "SountFont Version: " << version_tag_str(sf_version) << "\n";
	os << "Target Sound Engine: " << target_sound_engine << "\n";
	os << "SountFont Bank Name: " << sf_bank_name << "\n";
	if (sound_rom_name && sound_rom_version) {
		os << "Sound ROM(ver.): " << *sound_rom_name
		   << " (v" << version_tag_str(*sound_rom_version) << ")\n";
	}
	if (creation_date) {
		os << "Creation Date: " << *creation_date << "\n";
	}
	if (author) {
		os << "Author(s): " << *author << "\n";
	}
	if (target_product) {
		os << "Target Product: " << *target_product << "\n";
	}
	if (copyright_msg) {
		os << "Copyright: " << *copyright_msg << "\n";
	}
	if (comments) {
		os << "Comments: " << *comments << "\n";
	}
	if (sf_tools) {
		os << "Used Tools: " << *sf_tools << "\n";
	}
}