#pragma once

#include "sfpresetzone.hpp"
#include "sfhandleinterface.hpp"
#include <functional>
#include <optional>
#include <string>
#include <cstdint>

namespace sflib {
	class SfPreset {
	public:
		SfPreset(PresetHandle handle);

		PresetHandle GetHandle() const { return self_handle; }
		std::string GetName() const { return preset_name; };
		SfPresetZone& GetZone(PZoneHandle zone_handle);
		SfPresetZone& GetGlobalZone();
		SfPresetZone& NewZone();

		void RemoveZone(PZoneHandle zone_handle);

		auto FindZone(std::function<bool(const SfPresetZone&)> pred)
		-> std::optional<PZoneHandle>;

		auto FindZones(std::function<bool(const SfPresetZone&)> pred)
		-> std::vector<PZoneHandle>;

		SfPreset& SetPresetNumber(std::uint16_t x);
		SfPreset& SetBankNumber(std::uint16_t x);
		SfPreset& SetName(const std::string& x);

	private:
		PresetHandle self_handle;
		char preset_name[21] {};
		std::uint16_t preset_number;
		std::uint16_t bank_number;
		SfHandleInterface<SfPresetZone, PZoneHandle> zones;

		friend class SoundFontImpl;
		friend class PresetManager;
	};
}