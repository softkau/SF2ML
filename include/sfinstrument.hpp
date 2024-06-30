#ifndef SF2ML_SFINSTRUMENT_HPP_
#define SF2ML_SFINSTRUMENT_HPP_

#include "sfhandle.hpp"
#include "sfinstrumentzone.hpp"

#include <cstdint>
#include <functional>
#include <memory>
#include <optional>
#include <vector>
#include <string>
#include <string_view>

namespace SF2ML {
	class SfInstrument {
	public:
		SfInstrument(InstHandle handle);
		~SfInstrument();
		SfInstrument(const SfInstrument&) = delete;
		SfInstrument(SfInstrument&&) noexcept;
		SfInstrument& operator=(const SfInstrument&) = delete;
		SfInstrument& operator=(SfInstrument&&) noexcept;

		InstHandle GetHandle() const;
		SfInstrumentZone& GetZone(IZoneHandle zone_handle);
		SfInstrumentZone& GetGlobalZone();
		SfInstrumentZone& NewZone();
		std::uint32_t CountZones(bool count_empty=false) const;
		auto AllZoneHandles() const -> std::vector<IZoneHandle>;

		void ForEachZone(std::function<void(SfInstrumentZone&)> pred);
		void ForEachZone(std::function<void(const SfInstrumentZone&)> pred) const;

		void RemoveZone(IZoneHandle zone_handle);

		auto FindZone(std::function<bool(const SfInstrumentZone&)> pred)
		-> std::optional<IZoneHandle>;

		auto FindZones(std::function<bool(const SfInstrumentZone&)> pred)
		-> std::vector<IZoneHandle>;

		SfInstrument& SetName(std::string_view x);
		std::string GetName() const;

	private:
		std::unique_ptr<class SfInstrumentImpl> pimpl;
	};
}

#endif