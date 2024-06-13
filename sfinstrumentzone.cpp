#include "sfinstrumentzone.hpp"
#include "sample_manager.hpp"
#include <cmath>
#include <climits>

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

#define IZONE_S16_PLAIN_GETTER_IMPL(GeneratorType, DefaultBits) \
	auto SfInstrumentZone::Get##GeneratorType() const -> std::int16_t { \
		if (HasGenerator(SfGen##GeneratorType)) { \
			return generators[SfGen##GeneratorType].sh_amount; \
		} else { \
			return DefaultBits; \
		} \
	}

#define IZONE_TIME_CENT_GETTER_IMPL(GeneratorType, DefaultBits) \
	auto SfInstrumentZone::Get##GeneratorType() const -> double { \
		if (HasGenerator(SfGen##GeneratorType)) { \
			return TimeCentToSeconds(generators[SfGen##GeneratorType].sh_amount); \
		} else { \
			return TimeCentToSeconds(DefaultBits); \
		} \
	}

#define IZONE_ABSL_CENT_GETTER_IMPL(GeneratorType, DefaultBits) \
	auto SfInstrumentZone::Get##GeneratorType() const -> double { \
		if (HasGenerator(SfGen##GeneratorType)) { \
			return AbsoluteCentToHertz(generators[SfGen##GeneratorType].sh_amount); \
		} else { \
			return AbsoluteCentToHertz(DefaultBits); \
		} \
	}

#define IZONE_RANGE_GETTER_IMPL(GeneratorType) \
	auto SfInstrumentZone::Get##GeneratorType() const -> Ranges<std::uint8_t> { \
		if (HasGenerator(SfGen##GeneratorType)) { \
			return Ranges<std::uint8_t> { \
				.start = generators[SfGen##GeneratorType].ranges.by_lo, \
				.end   = generators[SfGen##GeneratorType].ranges.by_hi \
			}; \
		} else { \
			return { 0, 127 }; \
		} \
	}

#define IZONE_S16_PLAIN_SETTER_IMPL(GeneratorType) \
	auto SfInstrumentZone::Set##GeneratorType(std::optional<std::int16_t> x) -> SfInstrumentZone& { \
		if (x.has_value()) { \
			generators[SfGen##GeneratorType].sh_amount = x.value(); \
			active_gens.set(SfGen##GeneratorType); \
		} else { \
			active_gens.reset(SfGen##GeneratorType); \
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
			generators[SfGen##GeneratorType].sh_amount = cent; \
			active_gens.set(SfGen##GeneratorType); \
		} else { \
			active_gens.reset(SfGen##GeneratorType); \
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
			generators[SfGen##GeneratorType].sh_amount = cent; \
			active_gens.set(SfGen##GeneratorType); \
		} else { \
			active_gens.reset(SfGen##GeneratorType); \
		} \
		return *this; \
	}

#define IZONE_RANGE_SETTER_IMPL(GeneratorType) \
	auto SfInstrumentZone::Set##GeneratorType(std::optional<Ranges<std::uint8_t>> x) -> SfInstrumentZone& { \
		if (x.has_value()) { \
			generators[SfGen##GeneratorType].ranges.by_lo = x->start; \
			generators[SfGen##GeneratorType].ranges.by_hi = x->end; \
			active_gens.set(SfGen##GeneratorType); \
		} else { \
			active_gens.reset(SfGen##GeneratorType); \
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

auto SfInstrumentZone::GetSampleHandle() const -> std::optional<SfHandle> {
	return sample;
}

auto SfInstrumentZone::GetSampleModes() const -> LoopMode {
	if (HasGenerator(SfGenSampleModes)) {
		switch (generators[SfGenSampleModes].w_amount) {
			case 0: return LoopMode::NoLoop;
			case 1: case 2: return LoopMode::Loop;
			case 3: return LoopMode::LoopWithRemainder;
			default: return LoopMode::NoLoop;
		}
	} else {
		return LoopMode::NoLoop;
	}
}

auto SfInstrumentZone::SetSampleHandle(std::optional<SfHandle> x) -> SfInstrumentZone& {
	if (x.has_value()) {
		sample = x;
		active_gens.set(SfGenSampleID);
	} else {
		active_gens.reset(SfGenSampleID);
	}
	return *this;
}

auto SfInstrumentZone::SetSampleModes(std::optional<LoopMode> x) -> SfInstrumentZone& {
	if (x.has_value()) {
		active_gens.set(SfGenSampleModes);
		switch (x.value()) {
			case LoopMode::NoLoop:
				generators[SfGenSampleModes].w_amount = 0;
				break;
			case LoopMode::Loop:
				generators[SfGenSampleModes].w_amount = 1;
				break;
			case LoopMode::LoopWithRemainder:
				generators[SfGenSampleModes].w_amount = 3;
				break;
		}
	} else {
		active_gens.reset(SfGenSampleModes);
	}
	return *this;
}

DWORD SfInstrumentZone::RequiredSize() const {
	return active_gens.count() * sizeof(spec::SfInstGenList);
}

SfInstrumentZone& SfInstrumentZone::CopyProperties(const SfInstrumentZone& zone)
{
	active_gens = zone.active_gens;
	generators  = zone.generators;
	sample      = zone.sample;
	return *this;
}

SfInstrumentZone& SfInstrumentZone::MoveProperties(SfInstrumentZone&& zone) {
	active_gens = std::forward<decltype(active_gens)>(zone.active_gens);
	generators  = std::forward<decltype(generators)>(zone.generators);
	sample      = zone.sample;
	return *this;
}

SflibError SfInstrumentZone::SerializeGenerators(BYTE* dst, BYTE** end, const SampleManager& sample_manager) const {
	BYTE* pos = dst;
	DWORD gen_count = 0;
	const auto append_generator = [&pos, &gen_count](SFGenerator type, spec::GenAmountType amt) {
		spec::SfInstGenList bits;
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
			|| gen_type == SfGenSampleID
			|| !active_gens[gen_type]
		) {
			continue;
		}
		append_generator(static_cast<SFGenerator>(gen_type), generators[gen_type]);
	}
	if (active_gens[SfGenSampleID]) {
		spec::GenAmountType tmp;
		auto sample_id = sample_manager.GetSampleID(sample.value());
		if (sample_id.has_value() == false) {
			if (end) {
				*end = pos;
			}
			return SFLIB_NO_SUCH_SAMPLE;
		}
		tmp.w_amount = static_cast<WORD>(sample_id.value());
		append_generator(SfGenSampleID, tmp);
	}

	if (end) {
		*end = pos;
	}

	return SFLIB_SUCCESS;
}
