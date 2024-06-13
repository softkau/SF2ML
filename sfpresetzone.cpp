#include "sfpresetzone.hpp"
#include "instrument_manager.hpp"
#include <cmath>
#include <climits>
#include <utility>

using namespace sflib;

static inline double AbsoluteCentToHertz(SHORT cent) {
	return cent == 13500 ? 20000.0 : std::pow(2, cent/1200.0) * 8.176;
}

static inline SHORT HertzToAbsoluteCent(double hz) {
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

static inline double TimeCentToSeconds(SHORT cent) {
	return cent == -32768 ? 0.0 : std::pow(2, cent/1200.0);
}

static inline SHORT SecondsToTimeCent(double secs) {
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

#define PZONE_S16_PLAIN_GETTER_IMPL(GeneratorType, DefaultBits) \
	auto SfPresetZone::Get##GeneratorType() const -> std::int16_t { \
		if (HasGenerator(SfGen##GeneratorType)) { \
			return generators[SfGen##GeneratorType].sh_amount; \
		} else { \
			return DefaultBits; \
		} \
	}

#define PZONE_TIME_CENT_GETTER_IMPL(GeneratorType, DefaultBits) \
	auto SfPresetZone::Get##GeneratorType() const -> double { \
		if (HasGenerator(SfGen##GeneratorType)) { \
			return TimeCentToSeconds(generators[SfGen##GeneratorType].sh_amount); \
		} else { \
			return TimeCentToSeconds(DefaultBits); \
		} \
	}

#define PZONE_ABSL_CENT_GETTER_IMPL(GeneratorType, DefaultBits) \
	auto SfPresetZone::Get##GeneratorType() const -> double { \
		if (HasGenerator(SfGen##GeneratorType)) { \
			return AbsoluteCentToHertz(generators[SfGen##GeneratorType].sh_amount); \
		} else { \
			return AbsoluteCentToHertz(DefaultBits); \
		} \
	}

#define PZONE_RANGE_GETTER_IMPL(GeneratorType) \
	auto SfPresetZone::Get##GeneratorType() const -> Ranges<std::uint8_t> { \
		if (HasGenerator(SfGen##GeneratorType)) { \
			return Ranges<std::uint8_t> { \
				.start = generators[SfGen##GeneratorType].ranges.by_lo, \
				.end   = generators[SfGen##GeneratorType].ranges.by_hi \
			}; \
		} else { \
			return { 0, 127 }; \
		} \
	}

#define PZONE_S16_PLAIN_SETTER_IMPL(GeneratorType) \
	auto SfPresetZone::Set##GeneratorType(std::optional<std::int16_t> x) -> SfPresetZone& { \
		if (x.has_value()) { \
			generators[SfGen##GeneratorType].sh_amount = x.value(); \
			active_gens.set(SfGen##GeneratorType); \
		} else { \
			active_gens.reset(SfGen##GeneratorType); \
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
			generators[SfGen##GeneratorType].sh_amount = cent; \
			active_gens.set(SfGen##GeneratorType); \
		} else { \
			active_gens.reset(SfGen##GeneratorType); \
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
			generators[SfGen##GeneratorType].sh_amount = cent; \
			active_gens.set(SfGen##GeneratorType); \
		} else { \
			active_gens.reset(SfGen##GeneratorType); \
		} \
		return *this; \
	}

#define PZONE_RANGE_SETTER_IMPL(GeneratorType) \
	auto SfPresetZone::Set##GeneratorType(std::optional<Ranges<std::uint8_t>> x) -> SfPresetZone& { \
		if (x.has_value()) { \
			generators[SfGen##GeneratorType].ranges.by_lo = x->start; \
			generators[SfGen##GeneratorType].ranges.by_hi = x->end; \
			active_gens.set(SfGen##GeneratorType); \
		} else { \
			active_gens.reset(SfGen##GeneratorType); \
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

auto SfPresetZone::GetInstrument() const -> std::optional<SfHandle> {
	return instrument;
}

auto SfPresetZone::SetInstrument(std::optional<SfHandle> x) -> SfPresetZone& {
	if (x.has_value()) {
		active_gens.set(SfGenInstrument);
		instrument = x;
	} else {
		active_gens.reset(SfGenInstrument);
	}
	return *this;
}

DWORD SfPresetZone::RequiredSize() const {
	return active_gens.count() * sizeof(spec::SfGenList);
}

SfPresetZone& SfPresetZone::CopyProperties(const SfPresetZone& zone)
{
	active_gens = zone.active_gens;
	generators  = zone.generators;
	instrument  = zone.instrument;
	return *this;
}

SfPresetZone& SfPresetZone::MoveProperties(SfPresetZone&& zone) {
	active_gens = std::forward<decltype(active_gens)>(zone.active_gens);
	generators  = std::forward<decltype(generators)>(zone.generators);
	instrument  = zone.instrument;
	return *this;
}

SflibError SfPresetZone::SerializeGenerators(BYTE* dst, BYTE** end, const InstrumentManager& inst_manager) const {
	BYTE* pos = dst;
	DWORD gen_count = 0;
	const auto append_generator = [&pos, &gen_count](SFGenerator type, spec::GenAmountType amt) {
		spec::SfGenList bits;
		bits.sf_gen_oper = type;
		bits.gen_amount = amt;
		std::memcpy(pos, &bits, sizeof(bits));
		pos += sizeof(bits);
		gen_count++;
	};

	if (active_gens[SfGenKeyRange]) {
		append_generator(SfGenKeyRange, generators[SfGenKeyRange]);
	}
	if (active_gens[SfGenVelRange]) {
		append_generator(SfGenVelRange, generators[SfGenVelRange]);
	}
	
	for (WORD gen_type = 0; gen_type < SfGenEndOper; gen_type++) {
		if (   gen_type == SfGenKeyRange
			|| gen_type == SfGenVelRange
			|| gen_type == SfGenInstrument
			|| !active_gens[gen_type]
		) {
			continue;
		}
		append_generator(static_cast<SFGenerator>(gen_type), generators[gen_type]);
	}
	if (active_gens[SfGenInstrument]) {
		spec::GenAmountType tmp;
		auto inst_id = inst_manager.GetInstID(instrument.value());
		if (inst_id.has_value() == false) {
			if (end) {
				*end = pos;
			}
			return SFLIB_NO_SUCH_INSTRUMENT;
		}
		tmp.w_amount = static_cast<WORD>(inst_id.value());
		append_generator(SfGenInstrument, tmp);
	}

	if (end) {
		*end = pos;
	}

	return SFLIB_SUCCESS;
}
