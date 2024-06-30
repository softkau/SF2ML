#ifndef SF2ML_SFPRESET_HPP_
#define SF2ML_SFPRESET_HPP_

#include "sfhandle.hpp"
#include "sfpresetzone.hpp"

#include <cstdint>
#include <functional>
#include <memory>
#include <optional>
#include <vector>
#include <string>
#include <string_view>

namespace SF2ML {
	class SfPreset {
	public:
		SfPreset(PresetHandle handle);
		~SfPreset();
		SfPreset(const SfPreset&) = delete;
		SfPreset(SfPreset&&) noexcept;
		SfPreset& operator=(const SfPreset&) = delete;
		SfPreset& operator=(SfPreset&&) noexcept;

		PresetHandle GetHandle() const;
		SfPresetZone& GetZone(PZoneHandle zone_handle);
		SfPresetZone& GetGlobalZone();
		SfPresetZone& NewZone();
		std::uint32_t CountZones(bool count_empty=false) const;
		auto AllZoneHandles() const -> std::vector<PZoneHandle>;

		void ForEachZone(std::function<void(SfPresetZone&)> pred);
		void ForEachZone(std::function<void(const SfPresetZone&)> pred) const;

		void RemoveZone(PZoneHandle zone_handle);

		auto FindZone(std::function<bool(const SfPresetZone&)> pred)
		-> std::optional<PZoneHandle>;

		auto FindZones(std::function<bool(const SfPresetZone&)> pred)
		-> std::vector<PZoneHandle>;

		SfPreset& SetPresetNumber(std::uint16_t x);
		SfPreset& SetBankNumber(std::uint16_t x);
		SfPreset& SetName(std::string_view x);

		std::uint16_t GetPresetNumber() const;
		std::uint16_t GetBankNumber() const;
		std::string GetName() const;

	private:
		std::unique_ptr<class SfPresetImpl> pimpl;
	};
}

#endif