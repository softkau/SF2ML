#pragma once

#include "sfspec.hpp"
#include <string>
#include <optional>

namespace sflib {
	struct VersionTag { unsigned major = 0, minor = 0; };

	class SfInfo {
	public:
		DWORD InfoSize() const;
		SflibError Serialize(BYTE* dst, BYTE** end=nullptr);

		SfInfo& SetSoundFontVersion(std::uint16_t major, std::uint16_t minor);
		SfInfo& SetSoundEngine(const std::string& sound_engine_name);
		SfInfo& SetBankName(const std::string& bank_name);

	private:
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

		friend class SoundFontImpl;
	};
}