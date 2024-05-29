#include "info_manager.hpp"
#include "sfmap.hpp"
#include <cstring>
#include <cstddef>
#include <iostream>

using namespace sflib;

static void DefaultLogger(const std::string& msg) {
	std::cout << "[SFLIB]" << msg << "\n";
}

InfoManager::InfoManager(const SfbkMap::Info& info) : logger(DefaultLogger) {
	if (!info.ifil || !info.isng || !info.inam) {
		status = SFLIB_FAILED;
		return;
	}

	ChunkHead ck_head;
	auto validate_zstr = [&ck_head](const BYTE* ck_data) -> bool {
		return ck_data[ck_head.ck_size - 1] == 0x00;
	};

	// ./ifil
	std::memcpy(&ck_head, info.ifil, sizeof(ck_head));
	if (ck_head.ck_size != 4) {
		status = SFLIB_INVALID_CK_SIZE;
		return;
	}
	SfVersionTag ver;
	std::memcpy(&ver, info.ifil + 8, sizeof(ver));
	sf_version.major = ver.w_major;
	sf_version.minor = ver.w_minor;

	// ./isng
	std::memcpy(&ck_head, info.isng, sizeof(ck_head));
	if (validate_zstr(info.isng + 8)) {
		target_sound_engine.assign(reinterpret_cast<const char*>(info.isng + 8));
	} else {
		target_sound_engine.assign("EMU8000");
	}

	// ./inam
	std::memcpy(&ck_head, info.inam, sizeof(ck_head));
	if (validate_zstr(info.inam + 8)) {
		sf_bank_name.assign(reinterpret_cast<const char*>(info.inam + 8));
	} else {
		status = SFLIB_ZSTR_CHECK_FAILED;
		return;
	}

	// ./irom(optional)
	if (info.irom) {
		std::memcpy(&ck_head, info.irom, sizeof(ck_head));
		if (validate_zstr(info.irom + 8)) {
			sound_rom_name = std::string(reinterpret_cast<const char*>(info.irom + 8));
		}
	}
	
	// ./iver(optional)
	if (info.iver) {
		std::memcpy(&ck_head, info.iver, sizeof(ck_head));
		if (ck_head.ck_size == 4) {
			SfVersionTag ver;
			std::memcpy(&ver, info.iver + 8, sizeof(ver));
			sound_rom_version = VersionTag {
				.major = ver.w_major,
				.minor = ver.w_minor
			};
		}
	}

	// ./ICRD(optional)
	if (info.icrd) {
		std::memcpy(&ck_head, info.icrd, sizeof(ck_head));
		if (validate_zstr(info.icrd + 8)) {
			creation_date = std::string(reinterpret_cast<const char*>(info.icrd + 8));
		}
	}

	// ./IENG(optional)
	if (info.ieng) {
		std::memcpy(&ck_head, info.ieng, sizeof(ck_head));
		if (validate_zstr(info.ieng + 8)) {
			author = std::string(reinterpret_cast<const char*>(info.ieng + 8));
		}
	}

	// ./IPRD(optional)
	if (info.iprd) {
		std::memcpy(&ck_head, info.iprd, sizeof(ck_head));
		if (validate_zstr(info.iprd + 8)) {
			target_product = std::string(reinterpret_cast<const char*>(info.iprd + 8));
		}
	}

	// ./ICOP(optional)
	if (info.icop) {
		std::memcpy(&ck_head, info.icop, sizeof(ck_head));
		if (validate_zstr(info.icop + 8)) {
			copyright_msg = std::string(reinterpret_cast<const char*>(info.icop + 8));
		}
	}

	// ./ICMT(optional)
	if (info.icmt) {
		std::memcpy(&ck_head, info.icmt, sizeof(ck_head));
		if (validate_zstr(info.icmt + 8)) {
			comments = std::string(reinterpret_cast<const char*>(info.icmt + 8));
		}
	}

	// ./ISFT(optional)
	if (info.isft) {
		std::memcpy(&ck_head, info.isft, sizeof(ck_head));
		if (validate_zstr(info.isft + 8)) {
			sf_tools = std::string(reinterpret_cast<const char*>(info.isft + 8));
		}
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