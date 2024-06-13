#pragma once

#include "sfinstrumentzone.hpp"
#include "sfhandle.hpp"
#include <functional>
#include <optional>
#include <string>

namespace sflib {
	class SfInstrument {
	public:
		SfInstrument(SfHandle handle);

		SfHandle GetHandle() const { return self_handle; }
		std::string GetName() const { return inst_name; }
		SfInstrumentZone& GetZone(SfHandle zone_handle);
		SfInstrumentZone& GetGlobalZone();
		SfInstrumentZone& NewZone();

		void RemoveZone(SfHandle zone_handle);

		auto FindZone(std::function<bool(const SfInstrumentZone&)> pred)
		-> std::optional<SfHandle>;

		auto FindZones(std::function<bool(const SfInstrumentZone&)> pred)
		-> std::vector<SfHandle>;

		SfInstrument& SetName(const std::string& x);

	private:
		SfHandle self_handle;
		char inst_name[21] {};
		SfHandleInterface<SfInstrumentZone> zones;

		friend class SoundFontImpl;
		friend class InstrumentManager;
	};
}