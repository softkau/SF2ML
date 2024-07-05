#ifndef SF2ML_SFGENERATOR_HPP_
#define SF2ML_SFGENERATOR_HPP_

#include "sfspec.hpp"
#include "sfhandle.hpp"
#include <variant>
#include <cmath>

namespace SF2ML {

	/// This variant represents the offical sfGenList/sfInstGenList::gen_amount union.
	using SfGenAmount = std::variant<SHORT, WORD, InstHandle, SmplHandle, Ranges<BYTE>>;

	/// converts absolute cent(*defined in sfspec24) to hertz
	inline double AbsoluteCentToHertz(SHORT cent) {
		return cent == 13500 ? 20000.0 : std::pow(2, cent/1200.0) * 8.176;
	}

	/// converts hertz to absolute cent(*defined in sfspec24)
	inline SHORT HertzToAbsoluteCent(double hz) {
		if (hz <= 0.0) {
			return 0;
		}

		int32_t cent = std::round(std::log2(hz/8.176) * 1200.0);
		if (cent < 0) {
			return 0;
		} else if (cent > 13500) {
			return 13500;
		} else {
			return cent;
		}
	}

	/// converts time cent(*defined in sfspec24) to seconds
	inline double TimeCentToSeconds(SHORT cent) {
		return cent == -32768 ? 0.0 : std::pow(2, cent/1200.0);
	}

	/// converts seconds to time cent(*defined in sfspec24)
	inline SHORT SecondsToTimeCent(double secs) {
		if (secs > 0.0) {
			int32_t res = std::round(std::log2(secs) * 1200.0);
			if (res > std::numeric_limits<SHORT>::max()) {
				return std::numeric_limits<SHORT>::max();
			} else if (res < std::numeric_limits<SHORT>::min()) {
				return std::numeric_limits<SHORT>::min();
			} else {
				return static_cast<SHORT>(res);
			}
		} else {
			return std::numeric_limits<SHORT>::min();
		}
	}
}

#endif