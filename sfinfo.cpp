#include "sfinfo.hpp"
#include <cassert>

using namespace sflib;

DWORD SfInfo::InfoSize() const {
	return sizeof(ChunkHead) + sizeof(FOURCC)
		+ sizeof(ChunkHead) + sizeof(spec::SfVersionTag)
		+ sizeof(ChunkHead) + target_sound_engine.length() + 1
		+ sizeof(ChunkHead) + sf_bank_name.length() + 1
		+ (sound_rom_name ? sizeof(ChunkHead) + sound_rom_name->length() + 1 : 0)
		+ (sound_rom_version ? sizeof(ChunkHead) + sizeof(spec::SfVersionTag) : 0)
		+ ( creation_date ? sizeof(ChunkHead) + creation_date->length() + 1 : 0)
		+ (        author ? sizeof(ChunkHead) + author->length() + 1 : 0)
		+ (target_product ? sizeof(ChunkHead) + target_product->length() + 1 : 0)
		+ ( copyright_msg ? sizeof(ChunkHead) + copyright_msg->length() + 1 : 0)
		+ (      comments ? sizeof(ChunkHead) + comments->length() + 1 : 0)
		+ (      sf_tools ? sizeof(ChunkHead) + sf_tools->length() + 1 : 0);
}

SflibError SfInfo::Serialize(BYTE* dst, BYTE** end) {
	BYTE* pos = dst;

	std::memcpy(pos, "LIST", 4);
	BYTE* const size_ptr = pos + 4;
	std::memcpy(pos + 8, "INFO", 4);	
	pos += 12;

	auto serialize_zstr = [&pos](const char* fourcc, const std::string& name) {
		std::memcpy(pos, fourcc, 4);
		pos += 4;
		DWORD len = name.length() + 1; // std::min<std::size_t>(name.length(), 255);
		std::memcpy(pos, &len, sizeof(DWORD));
		pos += 4;
		std::memcpy(pos, name.c_str(), len-1);
		pos += len-1;
		*pos = 0;
		pos++;
	};
	auto serialize_vtag = [&pos](const char* fourcc, const VersionTag& vtag) {
		std::memcpy(pos, fourcc, 4);
		const DWORD sz = sizeof(spec::SfVersionTag);
		std::memcpy(pos + 4, &sz, sizeof(DWORD));
		pos += 8;
		spec::SfVersionTag res {
			.w_major = vtag.major,
			.w_minor = vtag.minor
		};
		std::memcpy(pos, &res, sizeof(res));
		pos += sizeof(res);
	};

	serialize_vtag("ifil", sf_version);
	serialize_zstr("isng", target_sound_engine);
	serialize_zstr("INAM", sf_bank_name);
	if (sound_rom_name) {
		serialize_zstr("irom", *sound_rom_name);
	}
	if (sound_rom_version) {
		serialize_vtag("iver", *sound_rom_version);
	}
	if (creation_date) {
		serialize_zstr("ICRD", *creation_date);
	}
	if (author) {
		serialize_zstr("IENG", *author);
	}
	if (target_product) {
		serialize_zstr("IPRD", *target_product);
	}
	if (copyright_msg) {
		serialize_zstr("ICOP", *copyright_msg);
	}
	if (comments) {
		serialize_zstr("ICMT", *comments);
	}
	if (sf_tools) {
		serialize_zstr("ISFT", *sf_tools);
	}

	DWORD size = pos - dst - sizeof(ChunkHead);
	assert(size + 8 == this->InfoSize());
	std::memcpy(size_ptr, &size, sizeof(size));

	if (end) {
		*end = pos;
	}
	return SFLIB_SUCCESS;
}

SfInfo& SfInfo::SetSoundFontVersion(std::uint16_t major, std::uint16_t minor) {
	sf_version.major = major;
	sf_version.minor = minor;
	return *this;
}

SfInfo& SfInfo::SetSoundEngine(const std::string& sound_engine_name) {
	this->target_sound_engine = sound_engine_name;
	return *this;
}

SfInfo& SfInfo::SetBankName(const std::string& bank_name) {
	this->sf_bank_name = bank_name;
	return *this;
}
