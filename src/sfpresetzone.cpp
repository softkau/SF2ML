#include <sfpresetzone.hpp>
#include <sfgenerator.hpp>

#include <cmath>
#include <climits>
#include <cassert>
#include <utility>
#include <bitset>
#include <array>

using namespace SF2ML;

namespace SF2ML {
	class SfPresetZoneImpl {
		friend SfPresetZone;
		PZoneHandle self_handle;
		std::bitset<SfGenEndOper> active_gens {};
		std::array<SfGenAmount, SfGenEndOper> generators;
	public:
		SfPresetZoneImpl(PZoneHandle handle) : self_handle{handle} {}
	};
}

SfPresetZone::SfPresetZone(PZoneHandle handle) {
	pimpl = std::make_unique<SfPresetZoneImpl>(handle);
}

SfPresetZone::~SfPresetZone() {

}

SfPresetZone& SfPresetZone::operator=(SfPresetZone&& rhs) noexcept {
	this->pimpl = std::move(rhs.pimpl);
	return *this;
}

SfPresetZone::SfPresetZone(SfPresetZone&& rhs) noexcept {
	this->pimpl = std::move(rhs.pimpl);
}

PZoneHandle SfPresetZone::GetHandle() const {
	return pimpl->self_handle;
}

bool SfPresetZone::IsEmpty() const noexcept {
	return pimpl->active_gens.count() == 0;
}

DWORD SfPresetZone::GeneratorCount() const noexcept {
	return pimpl->active_gens.count();
}
bool SfPresetZone::HasGenerator(SFGenerator type) const {
	assert(static_cast<WORD>(type) < SfGenEndOper);
	return pimpl->active_gens[static_cast<WORD>(type)];
}

bool SfPresetZone::HasModulator(SFModulator type) const {
	return false;
}

#define PZONE_S16_PLAIN_GETTER_IMPL(GeneratorType, DefaultBits) \
	auto SfPresetZone::Get##GeneratorType() const -> std::int16_t { \
		if (HasGenerator(SfGen##GeneratorType)) { \
			return std::get<SHORT>(pimpl->generators[SfGen##GeneratorType]); \
		} else { \
			return DefaultBits; \
		} \
	}

#define PZONE_TIME_CENT_GETTER_IMPL(GeneratorType, DefaultBits) \
	auto SfPresetZone::Get##GeneratorType() const -> double { \
		if (HasGenerator(SfGen##GeneratorType)) { \
			return TimeCentToSeconds(std::get<SHORT>(pimpl->generators[SfGen##GeneratorType])); \
		} else { \
			return TimeCentToSeconds(DefaultBits); \
		} \
	}

#define PZONE_ABSL_CENT_GETTER_IMPL(GeneratorType, DefaultBits) \
	auto SfPresetZone::Get##GeneratorType() const -> double { \
		if (HasGenerator(SfGen##GeneratorType)) { \
			return AbsoluteCentToHertz(std::get<SHORT>(pimpl->generators[SfGen##GeneratorType])); \
		} else { \
			return AbsoluteCentToHertz(DefaultBits); \
		} \
	}

#define PZONE_RANGE_GETTER_IMPL(GeneratorType) \
	auto SfPresetZone::Get##GeneratorType() const -> Ranges<std::uint8_t> { \
		if (HasGenerator(SfGen##GeneratorType)) { \
			return std::get<Ranges<BYTE>>(pimpl->generators[SfGen##GeneratorType]); \
		} else { \
			return { 0, 127 }; \
		} \
	}

#define PZONE_S16_PLAIN_SETTER_IMPL(GeneratorType) \
	auto SfPresetZone::Set##GeneratorType(std::optional<std::int16_t> x) -> SfPresetZone& { \
		if (x.has_value()) { \
			pimpl->generators[SfGen##GeneratorType] = x.value(); \
			pimpl->active_gens.set(SfGen##GeneratorType); \
		} else { \
			pimpl->active_gens.reset(SfGen##GeneratorType); \
		} \
		return *this; \
	}

#define PZONE_TIME_CENT_SETTER_IMPL(GeneratorType, MinVal, MaxVal) \
	auto SfPresetZone::Set##GeneratorType(std::optional<double> x) -> SfPresetZone& { \
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

#define PZONE_ABSL_CENT_SETTER_IMPL(GeneratorType, MinVal, MaxVal) \
	auto SfPresetZone::Set##GeneratorType(std::optional<double> x) -> SfPresetZone& { \
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

#define PZONE_RANGE_SETTER_IMPL(GeneratorType) \
	auto SfPresetZone::Set##GeneratorType(std::optional<Ranges<std::uint8_t>> x) -> SfPresetZone& { \
		if (x.has_value()) { \
			Ranges<BYTE> ranges { x->start, x->end }; \
			pimpl->generators[SfGen##GeneratorType] = ranges; \
			pimpl->active_gens.set(SfGen##GeneratorType); \
		} else { \
			pimpl->active_gens.reset(SfGen##GeneratorType); \
		} \
		return *this; \
	}

PZONE_S16_PLAIN_GETTER_IMPL(ModLfoToPitch,       0)
PZONE_S16_PLAIN_GETTER_IMPL(VibLfoToPitch,       0)
PZONE_S16_PLAIN_GETTER_IMPL(ModEnvToPitch,       0)
PZONE_ABSL_CENT_GETTER_IMPL(InitialFilterFc, 13500)
PZONE_S16_PLAIN_GETTER_IMPL(InitialFilterQ,      0)
PZONE_S16_PLAIN_GETTER_IMPL(ModLfoToFilterFc,    0)
PZONE_S16_PLAIN_GETTER_IMPL(ModEnvToFilterFc,    0)
PZONE_S16_PLAIN_GETTER_IMPL(ModLfoToVolume,      0)
PZONE_S16_PLAIN_GETTER_IMPL(ChorusEffectsSend,   0)
PZONE_S16_PLAIN_GETTER_IMPL(ReverbEffectsSend,   0)
PZONE_S16_PLAIN_GETTER_IMPL(Pan,                 0)
PZONE_TIME_CENT_GETTER_IMPL(DelayModLFO,    -12000)
PZONE_ABSL_CENT_GETTER_IMPL(FreqModLFO,          0)
PZONE_TIME_CENT_GETTER_IMPL(DelayVibLFO,    -12000)
PZONE_ABSL_CENT_GETTER_IMPL(FreqVibLFO,          0)  
PZONE_TIME_CENT_GETTER_IMPL(DelayModEnv,    -12000)
PZONE_TIME_CENT_GETTER_IMPL(AttackModEnv,   -12000)
PZONE_TIME_CENT_GETTER_IMPL(HoldModEnv,     -12000)
PZONE_TIME_CENT_GETTER_IMPL(DecayModEnv,    -12000)
PZONE_S16_PLAIN_GETTER_IMPL(SustainModEnv,       0)
PZONE_TIME_CENT_GETTER_IMPL(ReleaseModEnv,  -12000)
PZONE_S16_PLAIN_GETTER_IMPL(KeynumToModEnvHold,  0)
PZONE_S16_PLAIN_GETTER_IMPL(KeynumToModEnvDecay, 0)
PZONE_TIME_CENT_GETTER_IMPL(DelayVolEnv,    -12000)
PZONE_TIME_CENT_GETTER_IMPL(AttackVolEnv,   -12000)
PZONE_TIME_CENT_GETTER_IMPL(HoldVolEnv,     -12000)
PZONE_TIME_CENT_GETTER_IMPL(DecayVolEnv,    -12000)
PZONE_S16_PLAIN_GETTER_IMPL(SustainVolEnv,       0)
PZONE_TIME_CENT_GETTER_IMPL(ReleaseVolEnv,  -12000)
PZONE_S16_PLAIN_GETTER_IMPL(KeynumToVolEnvHold,  0)
PZONE_S16_PLAIN_GETTER_IMPL(KeynumToVolEnvDecay, 0)
PZONE_RANGE_GETTER_IMPL(KeyRange)
PZONE_RANGE_GETTER_IMPL(VelRange)
PZONE_S16_PLAIN_GETTER_IMPL(InitialAttenuation,  0)
PZONE_S16_PLAIN_GETTER_IMPL(CoarseTune,          0)
PZONE_S16_PLAIN_GETTER_IMPL(FineTune,            0)
PZONE_S16_PLAIN_GETTER_IMPL(ScaleTuning,       100)

PZONE_S16_PLAIN_SETTER_IMPL(ModLfoToPitch)
PZONE_S16_PLAIN_SETTER_IMPL(VibLfoToPitch)
PZONE_S16_PLAIN_SETTER_IMPL(ModEnvToPitch)
PZONE_ABSL_CENT_SETTER_IMPL(InitialFilterFc, 1500, 13500)
PZONE_S16_PLAIN_SETTER_IMPL(InitialFilterQ)
PZONE_S16_PLAIN_SETTER_IMPL(ModLfoToFilterFc)
PZONE_S16_PLAIN_SETTER_IMPL(ModEnvToFilterFc)
PZONE_S16_PLAIN_SETTER_IMPL(ModLfoToVolume)
PZONE_S16_PLAIN_SETTER_IMPL(ChorusEffectsSend)
PZONE_S16_PLAIN_SETTER_IMPL(ReverbEffectsSend)
PZONE_S16_PLAIN_SETTER_IMPL(Pan)
PZONE_TIME_CENT_SETTER_IMPL(DelayModLFO,   -12000, 5000)
PZONE_ABSL_CENT_SETTER_IMPL(FreqModLFO,     -1600, 4500)
PZONE_TIME_CENT_SETTER_IMPL(DelayVibLFO,   -12000, 5000)
PZONE_ABSL_CENT_SETTER_IMPL(FreqVibLFO,    -16000, 4500)  
PZONE_TIME_CENT_SETTER_IMPL(DelayModEnv,   -12000, 5000)
PZONE_TIME_CENT_SETTER_IMPL(AttackModEnv,  -12000, 8000)
PZONE_TIME_CENT_SETTER_IMPL(HoldModEnv,    -12000, 5000)
PZONE_TIME_CENT_SETTER_IMPL(DecayModEnv,   -12000, 8000)
PZONE_S16_PLAIN_SETTER_IMPL(SustainModEnv)
PZONE_TIME_CENT_SETTER_IMPL(ReleaseModEnv, -12000, 8000)
PZONE_S16_PLAIN_SETTER_IMPL(KeynumToModEnvHold)
PZONE_S16_PLAIN_SETTER_IMPL(KeynumToModEnvDecay)
PZONE_TIME_CENT_SETTER_IMPL(DelayVolEnv,   -12000, 5000)
PZONE_TIME_CENT_SETTER_IMPL(AttackVolEnv,  -12000, 8000)
PZONE_TIME_CENT_SETTER_IMPL(HoldVolEnv,    -12000, 5000)
PZONE_TIME_CENT_SETTER_IMPL(DecayVolEnv,   -12000, 8000)
PZONE_S16_PLAIN_SETTER_IMPL(SustainVolEnv)
PZONE_TIME_CENT_SETTER_IMPL(ReleaseVolEnv, -12000, 8000)
PZONE_S16_PLAIN_SETTER_IMPL(KeynumToVolEnvHold)
PZONE_S16_PLAIN_SETTER_IMPL(KeynumToVolEnvDecay)
PZONE_RANGE_SETTER_IMPL(KeyRange)
PZONE_RANGE_SETTER_IMPL(VelRange)
PZONE_S16_PLAIN_SETTER_IMPL(InitialAttenuation)
PZONE_S16_PLAIN_SETTER_IMPL(CoarseTune)
PZONE_S16_PLAIN_SETTER_IMPL(FineTune)
PZONE_S16_PLAIN_SETTER_IMPL(ScaleTuning)

auto SfPresetZone::GetInstrument() const -> std::optional<InstHandle> {
	return std::get<InstHandle>(pimpl->generators[SfGenInstrument]);
}

auto SfPresetZone::SetInstrument(std::optional<InstHandle> x) -> SfPresetZone& {
	if (x.has_value()) {
		pimpl->active_gens.set(SfGenInstrument);
		pimpl->generators[SfGenInstrument] = x.value();
	} else {
		pimpl->active_gens.reset(SfGenInstrument);
	}
	return *this;
}

DWORD SfPresetZone::RequiredSize() const {
	return pimpl->active_gens.count() * sizeof(spec::SfGenList);
}

auto SF2ML::SfPresetZone::SetGenerator(SFGenerator type, std::optional<SfGenAmount> amt) -> SfPresetZone& {
	if (amt.has_value()) {
		pimpl->active_gens.set(type);
		pimpl->generators[type] = amt.value();
	} else {
		pimpl->active_gens.reset(type);
	}
	return *this;
}

auto SF2ML::SfPresetZone::GetGenerator(SFGenerator type) const -> SfGenAmount {
	return pimpl->generators[type];
}

SfPresetZone& SfPresetZone::CopyProperties(const SfPresetZone& zone) {
	pimpl->active_gens = zone.pimpl->active_gens;
	pimpl->generators  = zone.pimpl->generators;
	return *this;
}

SfPresetZone& SfPresetZone::MoveProperties(SfPresetZone&& zone) {
	pimpl->active_gens = std::move(zone.pimpl->active_gens);
	pimpl->generators  = std::move(zone.pimpl->generators);
	return *this;
}

