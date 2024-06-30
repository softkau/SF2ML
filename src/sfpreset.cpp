#include <sfpreset.hpp>
#include "sfhandleinterface.hpp"

using namespace SF2ML;

namespace SF2ML {
	class SfPresetImpl {
		friend SfPreset;
		PresetHandle self_handle;
		char preset_name[21] {};
		std::uint16_t preset_number;
		std::uint16_t bank_number;
		SfHandleInterface<SfPresetZone, PZoneHandle> zones;
	public:
		SfPresetImpl(PresetHandle handle) : self_handle(handle) {}
	};
}

SfPreset::SfPreset(PresetHandle handle) {
	pimpl = std::make_unique<SfPresetImpl>(handle);
	
	// global zone
	pimpl->zones.NewItem();
}

SfPreset::~SfPreset() {
	
}

SfPreset::SfPreset(SfPreset&& rhs) noexcept {
	this->pimpl = std::move(rhs.pimpl);
}

SfPreset& SfPreset::operator=(SfPreset&& rhs) noexcept {
	this->pimpl = std::move(rhs.pimpl);
	return *this;
}

PresetHandle SfPreset::GetHandle() const {
	return pimpl->self_handle;
}

std::string SfPreset::GetName() const {
	return pimpl->preset_name;
}

SfPresetZone& SfPreset::GetZone(PZoneHandle zone_handle) {
	return *pimpl->zones.Get(zone_handle);
}

SfPresetZone& SfPreset::GetGlobalZone() {
	return *pimpl->zones.Get(PZoneHandle{0});
}

SfPresetZone& SfPreset::NewZone() {
	return pimpl->zones.NewItem();
}

std::uint32_t SfPreset::CountZones(bool count_empty) const {
	if (count_empty) {
		return pimpl->zones.Count();
	} else {
		return pimpl->zones.CountIf([](const SfPresetZone& x) { return !x.IsEmpty(); });
	}
}

auto SfPreset::AllZoneHandles() const -> std::vector<PZoneHandle> {
	auto [first, last] = pimpl->zones.GetAllHandles();
	return std::vector<PZoneHandle>(first, last);
}

void SfPreset::ForEachZone(std::function<void(SfPresetZone &)> pred) {
	for (auto& zone : pimpl->zones) {
		pred(zone);
	}
}

void SfPreset::ForEachZone(std::function<void(const SfPresetZone &)> pred) const {
	for (auto& zone : pimpl->zones) {
		pred(zone);
	}
}

void SfPreset::RemoveZone(PZoneHandle zone_handle) {
	pimpl->zones.Remove(zone_handle);
}

auto SfPreset::FindZone(std::function<bool(const SfPresetZone &)> pred) -> std::optional<PZoneHandle> {
	for (const auto& zone : pimpl->zones) {
		if (pred(zone)) {
			return zone.GetHandle();
		}
	}
	return std::nullopt;
}

auto SfPreset::FindZones(std::function<bool(const SfPresetZone &)> pred) -> std::vector<PZoneHandle> {
	std::vector<PZoneHandle> res;
	for (const auto& zone : pimpl->zones) {
		if (pred(zone)) {
			res.push_back(zone.GetHandle());
		}
	}
	return res;
}

SfPreset& SfPreset::SetPresetNumber(std::uint16_t x) {
	pimpl->preset_number = std::min<std::uint16_t>(x, 127);
	return *this;
}

SfPreset& SfPreset::SetBankNumber(std::uint16_t x) {
	pimpl->bank_number = std::min<std::uint16_t>(x, 0x3FFF);
	return *this;
}

SfPreset& SfPreset::SetName(std::string_view x) {
	std::memset(pimpl->preset_name, 0, 21);
	std::memcpy(pimpl->preset_name, x.data(), std::min<size_t>(x.length(), 20));
	return *this;
}

std::uint16_t SfPreset::GetPresetNumber() const {
	return pimpl->preset_number;
}

std::uint16_t SfPreset::GetBankNumber() const {
	return pimpl->bank_number;
}
