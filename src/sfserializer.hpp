#ifndef SF2ML_SFSERIALIZER_HPP_
#define SF2ML_SFSERIALIZER_HPP_

#include <sfinfo.hpp>
#include "sfcontainers.hpp"

namespace SF2ML::serializer {
	DWORD CalculateRiffSize(const SfInfo& infos,
							const PresetContainer& presets,
							const InstContainer& insts,
							const SmplContainer& smpls,
							unsigned z_zone);
	SF2MLError SerializeRiff(BYTE* dst,
							 BYTE** end,
							 const SfInfo& infos,
							 const PresetContainer& presets,
							 const InstContainer& insts,
							 const SmplContainer& smpls,
							 unsigned z_zone);
	SF2MLError SerializeInfos(BYTE* dst, BYTE** end, const SfInfo& src);
	SF2MLError SerializePresets(BYTE* dst, BYTE** end, const PresetContainer& src, const InstContainer& inst_info);
	SF2MLError SerializeInstruments(BYTE* dst, BYTE** end, const InstContainer& src, const SmplContainer& smpl_info);
	SF2MLError SerializeSDTA(BYTE* dst, BYTE** end, const SmplContainer& src, unsigned z_zone);
	SF2MLError SerializeSHDR(BYTE* dst, BYTE** end, const SmplContainer& src, unsigned z_zone);
	SF2MLError SerializeGenerators(BYTE* dst, BYTE** end, const SfPresetZone& src, const InstContainer& inst_info);
	SF2MLError SerializeGenerators(BYTE* dst, BYTE** end, const SfInstrumentZone& src, const SmplContainer& smpl_info);
}

#endif