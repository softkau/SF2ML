#include <sfinstrument.hpp>
#include "sfhandleinterface.hpp"

using namespace SF2ML;

namespace SF2ML {
	class SfInstrumentImpl {
		friend SfInstrument;
		InstHandle self_handle;
		char inst_name[21] {};
		SfHandleInterface<SfInstrumentZone, IZoneHandle> zones;
	public:
		SfInstrumentImpl(InstHandle handle) : self_handle(handle) {}
	};
}

SfInstrument::SfInstrument(InstHandle handle) {
	pimpl = std::make_unique<SfInstrumentImpl>(handle);

	// global zone
	pimpl->zones.NewItem();
}

SfInstrument::~SfInstrument() {
}

SF2ML::SfInstrument::SfInstrument(SfInstrument&& rhs) noexcept {
	this->pimpl = std::move(rhs.pimpl);
}

SfInstrument& SfInstrument::operator=(SfInstrument&& rhs) noexcept {
	this->pimpl = std::move(rhs.pimpl);
	return *this;
}

InstHandle SfInstrument::GetHandle() const {
	return pimpl->self_handle;
}

SfInstrumentZone& SfInstrument::GetZone(IZoneHandle zone_handle)
{
	return *pimpl->zones.Get(zone_handle);
}

SfInstrumentZone& SfInstrument::GetGlobalZone() {
	return *pimpl->zones.Get(IZoneHandle{0});
}

SfInstrumentZone& SfInstrument::NewZone() {
	return pimpl->zones.NewItem();
}

std::uint32_t SfInstrument::CountZones(bool count_empty) const {
	if (count_empty) {
		return pimpl->zones.Count();
	} else {
		return pimpl->zones.CountIf([](const SfInstrumentZone& x) { return !x.IsEmpty(); });
	}
}

auto SfInstrument::AllZoneHandles() const -> std::vector<IZoneHandle> {
	auto [first, last] = pimpl->zones.GetAllHandles();
	return std::vector<IZoneHandle>(first, last);
}

void SfInstrument::ForEachZone(std::function<void(SfInstrumentZone &)> pred) {
	for (auto& zone : pimpl->zones) {
		pred(zone);
	}
}

void SfInstrument::ForEachZone(std::function<void(const SfInstrumentZone &)> pred) const {
	for (auto& zone : pimpl->zones) {
		pred(zone);
	}
}

void SfInstrument::RemoveZone(IZoneHandle zone_handle) {
	pimpl->zones.Remove(zone_handle);
}

auto SfInstrument::FindZone(std::function<bool(const SfInstrumentZone &)> pred) -> std::optional<IZoneHandle> {
	for (const auto& zone : pimpl->zones) {
		if (pred(zone)) {
			return zone.GetHandle();
		}
	}
	return std::nullopt;
}

auto SfInstrument::FindZones(std::function<bool(const SfInstrumentZone &)> pred) -> std::vector<IZoneHandle> {
	std::vector<IZoneHandle> res;
	for (const auto& zone : pimpl->zones) {
		if (pred(zone)) {
			res.push_back(zone.GetHandle());
		}
	}
	return res;
}

SfInstrument& SfInstrument::SetName(std::string_view x) {
	std::memset(pimpl->inst_name, 0, 21);
	std::memcpy(pimpl->inst_name, x.data(), std::min<size_t>(x.length(), 20));
	return *this;
}

std::string SF2ML::SfInstrument::GetName() const {
	return pimpl->inst_name;
}
