#pragma once

#include "sfinstrumentzone.hpp"
#include "sfhandleinterface.hpp"
#include <functional>
#include <optional>
#include <string>

namespace sflib {
	class SfInstrument {
	public:
		SfInstrument(InstHandle handle);

		InstHandle GetHandle() const { return self_handle; }
		std::string GetName() const { return inst_name; }
		SfInstrumentZone& GetZone(IZoneHandle zone_handle);
		SfInstrumentZone& GetGlobalZone();
		SfInstrumentZone& NewZone();

		void RemoveZone(IZoneHandle zone_handle);

		auto FindZone(std::function<bool(const SfInstrumentZone&)> pred)
		-> std::optional<IZoneHandle>;

		auto FindZones(std::function<bool(const SfInstrumentZone&)> pred)
		-> std::vector<IZoneHandle>;

		SfInstrument& SetName(const std::string& x);

	private:
		InstHandle self_handle;
		char inst_name[21] {};
		SfHandleInterface<SfInstrumentZone, IZoneHandle> zones;

		friend class SoundFontImpl;
		friend class InstrumentManager;
	};
}