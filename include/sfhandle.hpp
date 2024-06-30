#ifndef SF2ML_SFHANDLE_HPP_
#define SF2ML_SFHANDLE_HPP_

#include <cstdint>

namespace SF2ML {

	struct SmplHandle {
		explicit SmplHandle(uint16_t value) : value{value} {};
		uint16_t value; // do not change in client side code
		auto operator<=>(const SmplHandle&) const = default;
	};

	struct InstHandle {
		explicit InstHandle(uint16_t value) : value{value} {};
		uint16_t value; // do not change in client side code
		auto operator<=>(const InstHandle&) const = default;
	};

	struct PresetHandle {
		explicit PresetHandle(uint32_t value) : value{value} {};
		uint32_t value; // do not change in client side code
		auto operator<=>(const PresetHandle&) const = default;
	};

	struct IZoneHandle {
		explicit IZoneHandle(uint32_t value) : value{value} {};
		uint32_t value; // do not change in client side code
		auto operator<=>(const IZoneHandle&) const = default;
	};

	struct PZoneHandle {
		explicit PZoneHandle(uint32_t value) : value{value} {};
		uint32_t value; // do not change in client side code
		auto operator<=>(const PZoneHandle&) const = default;
	};
}

#endif