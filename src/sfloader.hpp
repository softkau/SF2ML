#ifndef SF2ML_SFLOADER_HPP_
#define SF2ML_SFLOADER_HPP_

#include <sfspec.hpp>
#include <sfpresetzone.hpp>
#include <sfinstrumentzone.hpp>
#include <sfsample.hpp>
#include <sfinfo.hpp>
#include "sfcontainers.hpp"
#include "sfmap.hpp"

namespace SF2ML::loader {
	SF2MLError LoadSfbk(SfInfo& infos,
						PresetContainer& presets,
						InstContainer& insts,
						SmplContainer& smpls,
						const SfbkMap& sfbk);
	SF2MLError LoadInfos(SfInfo& infos, const SfbkMap& sfbk);
	SF2MLError LoadPresets(PresetContainer& presets, const SfbkMap& sfbk);
	SF2MLError LoadInstruments(InstContainer& insts, const SfbkMap& sfbk);
	SF2MLError LoadSamples(SmplContainer& smpls, const SfbkMap& sfbk);
	SF2MLError LoadGenerators(SfPresetZone& dst, const BYTE* buf, DWORD count);
	SF2MLError LoadGenerators(SfInstrumentZone& dst, const BYTE* buf, DWORD count);
	SF2MLError LoadModulators(SfPresetZone& dst, const BYTE* buf, DWORD count);
	SF2MLError LoadModulators(SfInstrumentZone& dst, const BYTE* buf, DWORD count);
}

#endif