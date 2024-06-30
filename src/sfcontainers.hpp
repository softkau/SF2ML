#ifndef SF2ML_SFCONTAINERS_HPP_
#define SF2ML_SFCONTAINERS_HPP_

#include "sfhandleinterface.hpp"
#include <sfpreset.hpp>
#include <sfpresetzone.hpp>
#include <sfinstrument.hpp>
#include <sfinstrumentzone.hpp>
#include <sfsample.hpp>

namespace SF2ML {
	using PresetContainer = SfHandleInterface<SfPreset, PresetHandle>;
	using InstContainer = SfHandleInterface<SfInstrument, InstHandle>;
	using SmplContainer = SfHandleInterface<SfSample, SmplHandle>;

	inline SampleBitDepth GetBitDepth(const SmplContainer& samples) {
		if (samples.Count() > 0) {
			SampleBitDepth bit_depth = samples.begin()->GetBitDepth();
			if (std::any_of(samples.begin(), samples.end(), [=](const SfSample& x) { return x.GetBitDepth() != bit_depth; })) {
				throw std::invalid_argument("argument <samples> is in invalid state: Mixed bit depth");
			}
			return bit_depth;
		} else {
			return SampleBitDepth::Signed16;
		}
	}
}

#endif