#include <sfml/sfpreset.hpp>

using namespace sflib;

SfPreset::SfPreset(PresetHandle handle) : self_handle(handle) {
	// global zone
	zones.NewItem();
}

SfPresetZone& SfPreset::GetZone(PZoneHandle zone_handle) {
	return *zones.Get(zone_handle);
}

SfPresetZone& SfPreset::GetGlobalZone() {
	return *zones.Get(PZoneHandle{0});
}

SfPresetZone& SfPreset::NewZone() {
	return zones.NewItem();
}

void SfPreset::RemoveZone(PZoneHandle zone_handle) {
	zones.Remove(zone_handle);
}

auto SfPreset::FindZone(std::function<bool(const SfPresetZone &)> pred) -> std::optional<PZoneHandle> {
	for (const auto& zone : zones) {
		if (pred(zone)) {
			return zone.GetHandle();
		}
	}
	return std::nullopt;
}

auto SfPreset::FindZones(std::function<bool(const SfPresetZone &)> pred) -> std::vector<PZoneHandle> {
	std::vector<PZoneHandle> res;
	for (const auto& zone : zones) {
		if (pred(zone)) {
			res.push_back(zone.GetHandle());
		}
	}
	return res;
}

SfPreset& SfPreset::SetPresetNumber(std::uint16_t x) {
	preset_number = std::min<std::uint16_t>(x, 127);
	return *this;
}

SfPreset& SfPreset::SetBankNumber(std::uint16_t x) {
	bank_number = std::min<std::uint16_t>(x, 0x3FFF);
	return *this;
}

SfPreset& SfPreset::SetName(const std::string& x) {
	std::memset(preset_name, 0, 21);
	std::memcpy(preset_name, x.c_str(), std::min<size_t>(x.length(), 20));
	return *this;
}
