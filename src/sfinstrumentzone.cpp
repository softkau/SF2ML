#include <sfinstrumentzone.hpp>
#include <sfgenerator.hpp>

#include <cmath>
#include <climits>
#include <cassert>
#include <utility>
#include <bitset>
#include <array>

using namespace SF2ML;

namespace SF2ML {
	class SfInstrumentZoneImpl {
		friend SfInstrumentZone;
		IZoneHandle self_handle;
		std::bitset<SfGenEndOper> active_gens {};
		std::array<SfGenAmount, SfGenEndOper> generators;
	public:
		SfInstrumentZoneImpl(IZoneHandle handle) : self_handle{handle} {}
	};
}

SfInstrumentZone::SfInstrumentZone(IZoneHandle handle) {
	pimpl = std::make_unique<SfInstrumentZoneImpl>(handle);
}

SfInstrumentZone::~SfInstrumentZone() {

}

SfInstrumentZone::SfInstrumentZone(SfInstrumentZone&& rhs) noexcept {
	this->pimpl = std::move(rhs.pimpl);
}

SfInstrumentZone& SfInstrumentZone::operator=(SfInstrumentZone&& rhs) noexcept {
	this->pimpl = std::move(rhs.pimpl);
	return *this;
}

IZoneHandle SfInstrumentZone::GetHandle() const {
	return pimpl->self_handle;
}

bool SfInstrumentZone::IsEmpty() const noexcept {
	return pimpl->active_gens.count() == 0;
}

DWORD SfInstrumentZone::GeneratorCount() const noexcept {
	return pimpl->active_gens.count();
}

bool SfInstrumentZone::HasGenerator(SFGenerator type) const {
	assert(static_cast<WORD>(type) < SfGenEndOper);
	return pimpl->active_gens[static_cast<WORD>(type)];
}

bool SfInstrumentZone::HasModulator(SFModulator type) const {
	return false;
}

#define IZONE_S16_PLAIN_GETTER_IMPL(GeneratorType, DefaultBits) \
	auto SfInstrumentZone::Get##GeneratorType() const -> std::int16_t { \
		if (HasGenerator(SfGen##GeneratorType)) { \
			return std::get<SHORT>(pimpl->generators[SfGen##GeneratorType]); \
		} else { \
			return DefaultBits; \
		} \
	}

#define IZONE_TIME_CENT_GETTER_IMPL(GeneratorType, DefaultBits) \
	auto SfInstrumentZone::Get##GeneratorType() const -> double { \
		if (HasGenerator(SfGen##GeneratorType)) { \
			return TimeCentToSeconds(std::get<SHORT>(pimpl->generators[SfGen##GeneratorType])); \
		} else { \
			return TimeCentToSeconds(DefaultBits); \
		} \
	}

#define IZONE_ABSL_CENT_GETTER_IMPL(GeneratorType, DefaultBits) \
	auto SfInstrumentZone::Get##GeneratorType() const -> double { \
		if (HasGenerator(SfGen##GeneratorType)) { \
			return AbsoluteCentToHertz(std::get<SHORT>(pimpl->generators[SfGen##GeneratorType])); \
		} else { \
			return AbsoluteCentToHertz(DefaultBits); \
		} \
	}

#define IZONE_RANGE_GETTER_IMPL(GeneratorType) \
	auto SfInstrumentZone::Get##GeneratorType() const -> Ranges<std::uint8_t> { \
		if (HasGenerator(SfGen##GeneratorType)) { \
			return std::get<Ranges<BYTE>>(pimpl->generators[SfGen##GeneratorType]); \
		} else { \
			return { 0, 127 }; \
		} \
	}

#define IZONE_S16_PLAIN_SETTER_IMPL(GeneratorType) \
	auto SfInstrumentZone::Set##GeneratorType(std::optional<std::int16_t> x) -> SfInstrumentZone& { \
		if (x.has_value()) { \
			pimpl->generators[SfGen##GeneratorType] = x.value(); \
			pimpl->active_gens.set(SfGen##GeneratorType); \
		} else { \
			pimpl->active_gens.reset(SfGen##GeneratorType); \
		} \
		return *this; \
	}

#define IZONE_TIME_CENT_SETTER_IMPL(GeneratorType, MinVal, MaxVal) \
	auto SfInstrumentZone::Set##GeneratorType(std::optional<double> x) -> SfInstrumentZone& { \
		if (x.has_value()) { \
			SHORT cent = SecondsToTimeCent(x.value()); \
			if (cent < (MinVal)) { \
				cent = (MinVal); \
			} else if (cent > (MaxVal)) { \
				cent = (MaxVal); \
			} \
			pimpl->generators[SfGen##GeneratorType] = cent; \
			pimpl->active_gens.set(SfGen##GeneratorType); \
		} else { \
			pimpl->active_gens.reset(SfGen##GeneratorType); \
		} \
		return *this; \
	}

#define IZONE_ABSL_CENT_SETTER_IMPL(GeneratorType, MinVal, MaxVal) \
	auto SfInstrumentZone::Set##GeneratorType(std::optional<double> x) -> SfInstrumentZone& { \
		if (x.has_value()) { \
			SHORT cent = HertzToAbsoluteCent(x.value()); \
			if (cent < (MinVal)) { \
				cent = (MinVal); \
			} else if (cent > (MaxVal)) { \
				cent = (MaxVal); \
			} \
			pimpl->generators[SfGen##GeneratorType] = cent; \
			pimpl->active_gens.set(SfGen##GeneratorType); \
		} else { \
			pimpl->active_gens.reset(SfGen##GeneratorType); \
		} \
		return *this; \
	}

#define IZONE_RANGE_SETTER_IMPL(GeneratorType) \
	auto SfInstrumentZone::Set##GeneratorType(std::optional<Ranges<std::uint8_t>> x) -> SfInstrumentZone& { \
		if (x.has_value()) { \
			Ranges<BYTE> ranges { x->start, x->end }; \
			pimpl->generators[SfGen##GeneratorType] = ranges; \
			pimpl->active_gens.set(SfGen##GeneratorType); \
		} else { \
			pimpl->active_gens.reset(SfGen##GeneratorType); \
		} \
		return *this; \
	}

IZONE_S16_PLAIN_GETTER_IMPL(ModLfoToPitch,       0)
IZONE_S16_PLAIN_GETTER_IMPL(VibLfoToPitch,       0)
IZONE_S16_PLAIN_GETTER_IMPL(ModEnvToPitch,       0)
IZONE_ABSL_CENT_GETTER_IMPL(InitialFilterFc, 13500)
IZONE_S16_PLAIN_GETTER_IMPL(InitialFilterQ,      0)
IZONE_S16_PLAIN_GETTER_IMPL(ModLfoToFilterFc,    0)
IZONE_S16_PLAIN_GETTER_IMPL(ModEnvToFilterFc,    0)
IZONE_S16_PLAIN_GETTER_IMPL(ModLfoToVolume,      0)
IZONE_S16_PLAIN_GETTER_IMPL(ChorusEffectsSend,   0)
IZONE_S16_PLAIN_GETTER_IMPL(ReverbEffectsSend,   0)
IZONE_S16_PLAIN_GETTER_IMPL(Pan,                 0)
IZONE_TIME_CENT_GETTER_IMPL(DelayModLFO,    -12000)
IZONE_ABSL_CENT_GETTER_IMPL(FreqModLFO,          0)
IZONE_TIME_CENT_GETTER_IMPL(DelayVibLFO,    -12000)
IZONE_ABSL_CENT_GETTER_IMPL(FreqVibLFO,          0)  
IZONE_TIME_CENT_GETTER_IMPL(DelayModEnv,    -12000)
IZONE_TIME_CENT_GETTER_IMPL(AttackModEnv,   -12000)
IZONE_TIME_CENT_GETTER_IMPL(HoldModEnv,     -12000)
IZONE_TIME_CENT_GETTER_IMPL(DecayModEnv,    -12000)
IZONE_S16_PLAIN_GETTER_IMPL(SustainModEnv,       0)
IZONE_TIME_CENT_GETTER_IMPL(ReleaseModEnv,  -12000)
IZONE_S16_PLAIN_GETTER_IMPL(KeynumToModEnvHold,  0)
IZONE_S16_PLAIN_GETTER_IMPL(KeynumToModEnvDecay, 0)
IZONE_TIME_CENT_GETTER_IMPL(DelayVolEnv,    -12000)
IZONE_TIME_CENT_GETTER_IMPL(AttackVolEnv,   -12000)
IZONE_TIME_CENT_GETTER_IMPL(HoldVolEnv,     -12000)
IZONE_TIME_CENT_GETTER_IMPL(DecayVolEnv,    -12000)
IZONE_S16_PLAIN_GETTER_IMPL(SustainVolEnv,       0)
IZONE_TIME_CENT_GETTER_IMPL(ReleaseVolEnv,  -12000)
IZONE_S16_PLAIN_GETTER_IMPL(KeynumToVolEnvHold,  0)
IZONE_S16_PLAIN_GETTER_IMPL(KeynumToVolEnvDecay, 0)
IZONE_RANGE_GETTER_IMPL(KeyRange)
IZONE_RANGE_GETTER_IMPL(VelRange)
IZONE_S16_PLAIN_GETTER_IMPL(InitialAttenuation,  0)
IZONE_S16_PLAIN_GETTER_IMPL(CoarseTune,          0)
IZONE_S16_PLAIN_GETTER_IMPL(FineTune,            0)
IZONE_S16_PLAIN_GETTER_IMPL(ScaleTuning,       100)

IZONE_S16_PLAIN_SETTER_IMPL(ModLfoToPitch)
IZONE_S16_PLAIN_SETTER_IMPL(VibLfoToPitch)
IZONE_S16_PLAIN_SETTER_IMPL(ModEnvToPitch)
IZONE_ABSL_CENT_SETTER_IMPL(InitialFilterFc, 1500, 13500)
IZONE_S16_PLAIN_SETTER_IMPL(InitialFilterQ)
IZONE_S16_PLAIN_SETTER_IMPL(ModLfoToFilterFc)
IZONE_S16_PLAIN_SETTER_IMPL(ModEnvToFilterFc)
IZONE_S16_PLAIN_SETTER_IMPL(ModLfoToVolume)
IZONE_S16_PLAIN_SETTER_IMPL(ChorusEffectsSend)
IZONE_S16_PLAIN_SETTER_IMPL(ReverbEffectsSend)
IZONE_S16_PLAIN_SETTER_IMPL(Pan)
IZONE_TIME_CENT_SETTER_IMPL(DelayModLFO,   -12000, 5000)
IZONE_ABSL_CENT_SETTER_IMPL(FreqModLFO,     -1600, 4500)
IZONE_TIME_CENT_SETTER_IMPL(DelayVibLFO,   -12000, 5000)
IZONE_ABSL_CENT_SETTER_IMPL(FreqVibLFO,    -16000, 4500)  
IZONE_TIME_CENT_SETTER_IMPL(DelayModEnv,   -12000, 5000)
IZONE_TIME_CENT_SETTER_IMPL(AttackModEnv,  -12000, 8000)
IZONE_TIME_CENT_SETTER_IMPL(HoldModEnv,    -12000, 5000)
IZONE_TIME_CENT_SETTER_IMPL(DecayModEnv,   -12000, 8000)
IZONE_S16_PLAIN_SETTER_IMPL(SustainModEnv)
IZONE_TIME_CENT_SETTER_IMPL(ReleaseModEnv, -12000, 8000)
IZONE_S16_PLAIN_SETTER_IMPL(KeynumToModEnvHold)
IZONE_S16_PLAIN_SETTER_IMPL(KeynumToModEnvDecay)
IZONE_TIME_CENT_SETTER_IMPL(DelayVolEnv,   -12000, 5000)
IZONE_TIME_CENT_SETTER_IMPL(AttackVolEnv,  -12000, 8000)
IZONE_TIME_CENT_SETTER_IMPL(HoldVolEnv,    -12000, 5000)
IZONE_TIME_CENT_SETTER_IMPL(DecayVolEnv,   -12000, 8000)
IZONE_S16_PLAIN_SETTER_IMPL(SustainVolEnv)
IZONE_TIME_CENT_SETTER_IMPL(ReleaseVolEnv, -12000, 8000)
IZONE_S16_PLAIN_SETTER_IMPL(KeynumToVolEnvHold)
IZONE_S16_PLAIN_SETTER_IMPL(KeynumToVolEnvDecay)
IZONE_RANGE_SETTER_IMPL(KeyRange)
IZONE_RANGE_SETTER_IMPL(VelRange)
IZONE_S16_PLAIN_SETTER_IMPL(InitialAttenuation)
IZONE_S16_PLAIN_SETTER_IMPL(CoarseTune)
IZONE_S16_PLAIN_SETTER_IMPL(FineTune)
IZONE_S16_PLAIN_SETTER_IMPL(ScaleTuning)

IZONE_S16_PLAIN_GETTER_IMPL(StartAddrsOffset,           0)
IZONE_S16_PLAIN_GETTER_IMPL(EndAddrsOffset,             0)
IZONE_S16_PLAIN_GETTER_IMPL(StartloopAddrsOffset,       0)
IZONE_S16_PLAIN_GETTER_IMPL(EndloopAddrsOffset,         0)
IZONE_S16_PLAIN_GETTER_IMPL(StartAddrsCoarseOffset,     0)
IZONE_S16_PLAIN_GETTER_IMPL(EndAddrsCoarseOffset,       0)
IZONE_S16_PLAIN_GETTER_IMPL(StartloopAddrsCoarseOffset, 0)
IZONE_S16_PLAIN_GETTER_IMPL(EndloopAddrsCoarseOffset,   0)
IZONE_S16_PLAIN_GETTER_IMPL(Keynum,            -1)
IZONE_S16_PLAIN_GETTER_IMPL(Velocity,          -1)
IZONE_S16_PLAIN_GETTER_IMPL(ExclusiveClass,     0)
IZONE_S16_PLAIN_GETTER_IMPL(OverridingRootKey, -1)

IZONE_S16_PLAIN_SETTER_IMPL(StartAddrsOffset)
IZONE_S16_PLAIN_SETTER_IMPL(EndAddrsOffset)
IZONE_S16_PLAIN_SETTER_IMPL(StartloopAddrsOffset)
IZONE_S16_PLAIN_SETTER_IMPL(EndloopAddrsOffset)
IZONE_S16_PLAIN_SETTER_IMPL(StartAddrsCoarseOffset)
IZONE_S16_PLAIN_SETTER_IMPL(EndAddrsCoarseOffset)
IZONE_S16_PLAIN_SETTER_IMPL(StartloopAddrsCoarseOffset)
IZONE_S16_PLAIN_SETTER_IMPL(EndloopAddrsCoarseOffset)
IZONE_S16_PLAIN_SETTER_IMPL(Keynum)
IZONE_S16_PLAIN_SETTER_IMPL(Velocity)
IZONE_S16_PLAIN_SETTER_IMPL(ExclusiveClass)
IZONE_S16_PLAIN_SETTER_IMPL(OverridingRootKey)

auto SfInstrumentZone::GetSampleHandle() const -> std::optional<SmplHandle> {
	return std::get<SmplHandle>(pimpl->generators[SfGenSampleID]);
}

auto SfInstrumentZone::GetSampleModes() const -> LoopMode {
	if (HasGenerator(SfGenSampleModes)) {
		WORD mode = std::get<WORD>(pimpl->generators[SfGenSampleModes]);
		switch (mode) {
			case 0: return LoopMode::NoLoop;
			case 1: case 2: return LoopMode::Loop;
			case 3: return LoopMode::LoopWithRemainder;
			default: return LoopMode::NoLoop;
		}
	} else {
		return LoopMode::NoLoop;
	}
}

auto SfInstrumentZone::SetSample(std::optional<SmplHandle> x) -> SfInstrumentZone& {
	if (x.has_value()) {
		pimpl->active_gens.set(SfGenSampleID);
		pimpl->generators[SfGenSampleID] = x.value();
	} else {
		pimpl->active_gens.reset(SfGenSampleID);
	}
	return *this;
}

auto SfInstrumentZone::SetSampleModes(std::optional<LoopMode> x) -> SfInstrumentZone& {
	if (x.has_value()) {
		pimpl->active_gens.set(SfGenSampleModes);
		switch (x.value()) {
			case LoopMode::NoLoop:
				pimpl->generators[SfGenSampleModes] = WORD(0);
				break;
			case LoopMode::Loop:
				pimpl->generators[SfGenSampleModes] = WORD(1);
				break;
			case LoopMode::LoopWithRemainder:
				pimpl->generators[SfGenSampleModes] = WORD(3);
				break;
		}
	} else {
		pimpl->active_gens.reset(SfGenSampleModes);
	}
	return *this;
}

DWORD SfInstrumentZone::RequiredSize() const {
	return pimpl->active_gens.count() * sizeof(spec::SfInstGenList);
}

auto SfInstrumentZone::SetGenerator(SFGenerator type, std::optional<SfGenAmount> amt) -> SfInstrumentZone& {
	if (amt.has_value()) {
		pimpl->active_gens.set(type);
		pimpl->generators[type] = amt.value();
	} else {
		pimpl->active_gens.reset(type);
	}
	return *this;
}

auto SfInstrumentZone::GetGenerator(SFGenerator type) const -> SfGenAmount {
	return pimpl->generators[type];
}

SfInstrumentZone& SfInstrumentZone::CopyProperties(const SfInstrumentZone& zone)
{
	pimpl->active_gens = zone.pimpl->active_gens;
	pimpl->generators  = zone.pimpl->generators;
	return *this;
}

SfInstrumentZone& SfInstrumentZone::MoveProperties(SfInstrumentZone&& zone) {
	pimpl->active_gens = std::move(zone.pimpl->active_gens);
	pimpl->generators  = std::move(zone.pimpl->generators);
	return *this;
}
