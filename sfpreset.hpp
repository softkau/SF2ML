#pragma once

#include "sfpresetzone.hpp"
#include "sfhandle.hpp"
#include <functional>
#include <optional>
#include <string>
#include <cstdint>

namespace sflib {
	class SfPreset {
	public:
		SfPreset(SfHandle handle);

		SfHandle GetHandle() const { return self_handle; }
		std::string GetName() const { return preset_name; };
		SfPresetZone& GetZone(SfHandle zone_handle);
		SfPresetZone& GetGlobalZone();
		SfPresetZone& NewZone();

		void RemoveZone(SfHandle zone_handle);

		auto FindZone(std::function<bool(const SfPresetZone&)> pred)
		-> std::optional<SfHandle>;

		auto FindZones(std::function<bool(const SfPresetZone&)> pred)
		-> std::vector<SfHandle>;

		SfPreset& SetPresetNumber(std::uint16_t x);
		SfPreset& SetBankNumber(std::uint16_t x);
		SfPreset& SetName(const std::string& x);

	private:
		SfHandle self_handle;
		char preset_name[21] {};
		std::uint16_t preset_number;
		std::uint16_t bank_number;
		SfHandleInterface<SfPresetZone> zones;

		friend class SoundFontImpl;
		friend class PresetManager;
	};
}