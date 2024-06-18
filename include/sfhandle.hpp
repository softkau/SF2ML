#pragma once

#include <vector>
#include <map>
#include <algorithm>
#include <optional>
#include <cassert>
#include <utility>
#include <limits>
#include <concepts>
#include "sfspec.hpp"

namespace sflib {

	struct SmplHandle {
		explicit SmplHandle(uint32_t value) : value{value} {};
		uint32_t value;
		auto operator<=>(const SmplHandle&) const = default;
	};

	struct InstHandle {
		explicit InstHandle(uint32_t value) : value{value} {};
		uint32_t value;
		auto operator<=>(const InstHandle&) const = default;
	};

	struct PresetHandle {
		explicit PresetHandle(uint32_t value) : value{value} {};
		uint32_t value;
		auto operator<=>(const PresetHandle&) const = default;
	};

	struct IZoneHandle {
		explicit IZoneHandle(uint32_t value) : value{value} {};
		uint32_t value;
		auto operator<=>(const IZoneHandle&) const = default;
	};

	struct PZoneHandle {
		explicit PZoneHandle(uint32_t value) : value{value} {};
		uint32_t value;
		auto operator<=>(const PZoneHandle&) const = default;
	};
}