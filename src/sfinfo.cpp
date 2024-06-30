#include <sfinfo.hpp>
#include <type_traits>
#include <cassert>
#include <optional>
#include <string>

using namespace SF2ML;

namespace SF2ML {
	class SfInfoImpl {
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

		friend SfInfo;
	};
}

namespace {
	std::uint16_t GetHash(std::string_view sv) {
		std::uint64_t h64 = std::hash<std::string_view>{}(sv);
		std::uint16_t h16 = (h64 & 0xFFFF)
						  ^ ((h64 >> 16) & 0xFFFF)
						  ^ ((h64 >> 32) & 0xFFFF)
						  ^ ((h64 >> 48) & 0xFFFF);
		return h16;
	}

	void SetStr255(std::string& dst, std::string_view sv) {
		constexpr std::size_t LIMIT = 255;
		const char* hex = "0123456789abcdef";

		if (sv.length() > LIMIT) {
			std::uint16_t h16 = GetHash(sv.substr(LIMIT - 4));
			char ah16[5] = {};
			ah16[0] = hex[(h16 >> 12) & 0xf];
			ah16[1] = hex[(h16 >>  8) & 0xf];
			ah16[2] = hex[(h16 >>  4) & 0xf];
			ah16[3] = hex[(h16 >>  0) & 0xf];

			dst = sv.substr(0, LIMIT - 4);
			dst.append(ah16);
		} else {
			dst = sv;
		}
	}

	void SetStr255(std::optional<std::string>& dst, std::optional<std::string_view> sv) {
		if (sv) {
			dst = "";
			SetStr255(*dst, *sv);
		} else {
			dst = std::nullopt;
		}
	}
}

SfInfo::SfInfo() {
	pimpl = std::make_unique<SfInfoImpl>();
}

SfInfo::~SfInfo() {
	
}

SfInfo::SfInfo(SfInfo&& rhs) noexcept {
	this->pimpl = std::move(rhs.pimpl);
}

SfInfo& SfInfo::operator=(SfInfo&& rhs) noexcept {
	this->pimpl = std::move(rhs.pimpl);
	return *this;
}

SfInfo& SfInfo::SetSoundFontVersion(VersionTag sf_version) {
	pimpl->sf_version = sf_version;
	return *this;
}

SfInfo& SfInfo::SetSoundEngine(std::string_view sound_engine_name) {
	SetStr255(pimpl->target_sound_engine, sound_engine_name);
	pimpl->target_sound_engine = sound_engine_name;
	return *this;
}

SfInfo& SfInfo::SetBankName(std::string_view bank_name) {
	SetStr255(pimpl->sf_bank_name, bank_name);
	return *this;
}

SfInfo& SfInfo::SetSoundRomName(std::optional<std::string_view> rom_name) {
	SetStr255(pimpl->sound_rom_name, rom_name);
	return *this;
}

SfInfo& SfInfo::SetSoundRomVersion(std::optional<VersionTag> rom_version) {
	pimpl->sound_rom_version = rom_version;
	return *this;
}

SfInfo& SfInfo::SetCreationDate(std::optional<std::string_view> date) {
	SetStr255(pimpl->creation_date, date);
	return *this;
}

SfInfo& SfInfo::SetAuthor(std::optional<std::string_view> author) {
	SetStr255(pimpl->author, author);
	return *this;
}

SfInfo& SfInfo::SetTargetProduct(std::optional<std::string_view> target_product) {
	SetStr255(pimpl->target_product, target_product);
	return *this;
}

SfInfo& SfInfo::SetCopyrightMessage(std::optional<std::string_view> message) {
	SetStr255(pimpl->copyright_msg, message);
	return *this;
}

SfInfo& SfInfo::SetComments(std::optional<std::string_view> comments) {
	SetStr255(pimpl->comments, comments);
	return *this;
}

SfInfo& SfInfo::SetToolUsed(std::optional<std::string_view> sf_tools) {
	SetStr255(pimpl->sf_tools, sf_tools);
	return *this;
}

auto SfInfo::GetSoundFontVersion() const -> VersionTag {
	return pimpl->sf_version;
}

auto SfInfo::GetSoundEngine() const -> std::string {
	return pimpl->target_sound_engine;
}

auto SfInfo::GetBankName() const -> std::string {
	return pimpl->sf_bank_name;
}

auto SfInfo::GetSoundRomName() const -> std::optional<std::string> {
	return pimpl->sound_rom_name;
}

auto SfInfo::GetSoundRomVersion() const -> std::optional<VersionTag> {
	return pimpl->sound_rom_version;
}

auto SfInfo::GetCreationDate() const -> std::optional<std::string> {
	return pimpl->creation_date;
}

auto SfInfo::GetAuthor() const -> std::optional<std::string> {
	return pimpl->author;
}

auto SfInfo::GetTargetProduct() const -> std::optional<std::string> {
	return pimpl->target_product;
}

auto SfInfo::GetCopyrightMessage() const -> std::optional<std::string> {
	return pimpl->copyright_msg;
}

auto SfInfo::GetComments() const -> std::optional<std::string> {
	return pimpl->comments;
}

auto SfInfo::GetToolUsed() const -> std::optional<std::string> {
	return pimpl->sf_tools;
}
