#ifndef SF2ML_SFINFO_HPP_
#define SF2ML_SFINFO_HPP_

#include "sfspec.hpp"
#include <string>
#include <string_view>
#include <optional>
#include <memory>

namespace SF2ML {
	struct VersionTag { std::uint16_t major = 2, minor = 1; };

	class SfInfo {
	public:
		SfInfo();
		~SfInfo();
		SfInfo(const SfInfo&) = delete;
		SfInfo(SfInfo&&) noexcept;
		SfInfo& operator=(const SfInfo&) = delete;
		SfInfo& operator=(SfInfo&&) noexcept;

		SfInfo& SetSoundFontVersion(VersionTag sf_version);
		SfInfo& SetSoundEngine(std::string_view sound_engine_name);
		SfInfo& SetBankName(std::string_view bank_name);
		SfInfo& SetSoundRomName(std::optional<std::string_view> rom_name);
		SfInfo& SetSoundRomVersion(std::optional<VersionTag> rom_version);
		SfInfo& SetCreationDate(std::optional<std::string_view> date);
		SfInfo& SetAuthor(std::optional<std::string_view> author);
		SfInfo& SetTargetProduct(std::optional<std::string_view> target_product);
		SfInfo& SetCopyrightMessage(std::optional<std::string_view> message);
		SfInfo& SetComments(std::optional<std::string_view> comments);
		SfInfo& SetToolUsed(std::optional<std::string_view> sf_tools);

		auto GetSoundFontVersion() const -> VersionTag;
		auto GetSoundEngine() const -> std::string;
		auto GetBankName() const -> std::string;
		auto GetSoundRomName() const -> std::optional<std::string>;
		auto GetSoundRomVersion() const -> std::optional<VersionTag>;
		auto GetCreationDate() const -> std::optional<std::string>;
		auto GetAuthor() const -> std::optional<std::string>;
		auto GetTargetProduct() const -> std::optional<std::string>;
		auto GetCopyrightMessage() const -> std::optional<std::string>;
		auto GetComments() const -> std::optional<std::string>;
		auto GetToolUsed() const -> std::optional<std::string>;

	private:
		std::unique_ptr<class SfInfoImpl> pimpl;
	};
}

#endif