#pragma once

#include <string>
#include <optional>
#include <functional>
#include <iostream>
#include "sfspec.hpp"

namespace sflib {
	class InfoManager {
	public:
		struct VersionTag { unsigned major = 0, minor = 0; };

		InfoManager(const BYTE* buf, std::size_t buf_size);

		// void SetSoundROM(const std::string& name, VersionTag ver);
		// void UnsetSoundROM();
		// void SetCreationDate(const std::string& time);
		// void UnsetCreationDate();

		void PrintInfo(std::ostream& os) const;
		SflibError Status() const { return status; }
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

		SflibError status = SFLIB_SUCCESS;
		std::function<void(const std::string&)> logger;
	};
};