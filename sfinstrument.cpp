#include "sfinstrument.hpp"

using namespace sflib;

SfInstrument::SfInstrument(SfHandle handle) : self_handle(handle) {
	// global zone
	zones.NewItem();
}

SfInstrumentZone& SfInstrument::GetZone(SfHandle zone_handle) {
	return *zones.Get(zone_handle);
}

SfInstrumentZone& SfInstrument::GetGlobalZone() {
	return *zones.Get(static_cast<SfHandle>(0));
}

SfInstrumentZone& SfInstrument::NewZone() {
	return zones.NewItem();
}

void SfInstrument::RemoveZone(SfHandle zone_handle) {
	zones.Remove(zone_handle);
}

auto SfInstrument::FindZone(std::function<bool(const SfInstrumentZone &)> pred) -> std::optional<SfHandle> {
	for (const auto& zone : zones) {
		if (pred(zone)) {
			return zone.GetHandle();
		}
	}
	return std::nullopt;
}

auto SfInstrument::FindZones(std::function<bool(const SfInstrumentZone &)> pred) -> std::vector<SfHandle> {
	std::vector<SfHandle> res;
	for (const auto& zone : zones) {
		if (pred(zone)) {
			res.push_back(zone.GetHandle());
		}
	}
	return res;
}

SfInstrument& SfInstrument::SetName(const std::string& x) {
	std::memset(inst_name, 0, 21);
	std::memcpy(inst_name, x.c_str(), std::min<size_t>(x.length(), 20));
	return *this;
}
