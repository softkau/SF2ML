#include "sfzone.hpp"
#include "sample_manager.hpp"
#include "instrument_manager.hpp"
#include <cmath>

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

ZoneBase::ZoneBase(const BYTE* gen_list_ptr, DWORD count) {
	// assuming SfGenList and SfInstGenList are compatible types
	for (size_t gen_ndx = 0; gen_ndx < count; gen_ndx++) {
		const BYTE* gen_ptr = gen_list_ptr + gen_ndx * sizeof(SfGenList);
		SfGenList gen;
		std::memcpy(&gen , gen_ptr, sizeof(SfGenList));

		generators[gen.sf_gen_oper] = gen.gen_amount;
		active_gens.set(gen.sf_gen_oper);
	}
}

#define PLAIN_S16DATA(GENERATOR) \
	(HasGenerator(GENERATOR) \
	? std::optional{ static_cast<int16_t>(generators[GENERATOR].sh_amount) } \
	: std::nullopt)

#define PLAIN_U16DATA(GENERATOR) \
	(HasGenerator(GENERATOR) \
	? std::optional{ static_cast<uint16_t>(generators[GENERATOR].w_amount) } \
	: std::nullopt)

auto ZoneBase::GetModLfoToPitch() const -> std::optional<int16_t> {
	return PLAIN_S16DATA(SfModLfoToPitch);
}

auto ZoneBase::GetVibLfoToPitch() const -> std::optional<int16_t> {
	return PLAIN_S16DATA(SfVibLfoToPitch);
}

auto ZoneBase::GetModEnvToPitch() const -> std::optional<int16_t> {
	return PLAIN_S16DATA(SfModEnvToPitch);
}

auto ZoneBase::GetInitialFilterFc() const -> std::optional<double> {
	if (HasGenerator(SfInitialFilterFc)) {
		SHORT cent = generators[SfInitialFilterFc].sh_amount;
		return AbsoluteCentToHertz(cent);
	} else {
		return std::nullopt;
	}
}

auto ZoneBase::GetInitialFilterQ() const -> std::optional<int16_t> {
	return PLAIN_S16DATA(SfInitialFilterQ);
}

auto ZoneBase::GetModLfoToFilterFc() const -> std::optional<int16_t> {
	return PLAIN_S16DATA(SfModLfoToFilterFc);
}

auto ZoneBase::GetModEnvToFilterFc() const -> std::optional<int16_t> {
	return PLAIN_S16DATA(SfModEnvToFilterFc);
}

auto ZoneBase::GetModLfoToVolume() const -> std::optional<int16_t> {
	return PLAIN_S16DATA(SfModLfoToVolume);
}

auto ZoneBase::GetChorusEffectsSend() const -> std::optional<int16_t> {
	return PLAIN_S16DATA(SfChorusEffectsSend);
}

auto ZoneBase::GetReverbEffectsSend() const -> std::optional<int16_t> {
	return PLAIN_S16DATA(SfReverbEffectsSend);
}

auto ZoneBase::GetPan() const -> std::optional<int16_t> {
	return PLAIN_S16DATA(SfPan);
}

auto ZoneBase::GetDelayModLFO() const -> std::optional<double> {
	if (HasGenerator(SfDelayModLFO)) {
		SHORT cent = generators[SfDelayModLFO].sh_amount;
		return TimeCentToSeconds(cent);
	} else {
		return std::nullopt;
	}
}

auto ZoneBase::GetFreqModLFO() const -> std::optional<double> {
	if (HasGenerator(SfFreqModLFO)) {
		SHORT cent = generators[SfFreqModLFO].sh_amount;
		return AbsoluteCentToHertz(cent);
	} else {
		return std::nullopt;
	}
}

auto ZoneBase::GetDelayVibLFO() const -> std::optional<double> {
	if (HasGenerator(SfDelayVibLFO)) {
		SHORT cent = generators[SfDelayVibLFO].sh_amount;
		return TimeCentToSeconds(cent);
	} else {
		return std::nullopt;
	}
}

auto ZoneBase::GetFreqVibLFO() const -> std::optional<double> {
	if (HasGenerator(SfFreqVibLFO)) {
		SHORT cent = generators[SfFreqVibLFO].sh_amount;
		return AbsoluteCentToHertz(cent);
	} else {
		return std::nullopt;
	}
}

auto ZoneBase::GetDelayModEnv() const -> std::optional<double> {
	if (HasGenerator(SfDelayModEnv)) {
		SHORT cent = generators[SfDelayModEnv].sh_amount;
		return TimeCentToSeconds(cent);
	} else {
		return std::nullopt;
	}
}

auto ZoneBase::GetAttackModEnv() const -> std::optional<double> {
	if (HasGenerator(SfAttackModEnv)) {
		SHORT cent = generators[SfAttackModEnv].sh_amount;
		return TimeCentToSeconds(cent);
	} else {
		return std::nullopt;
	}
}

auto ZoneBase::GetHoldModEnv() const -> std::optional<double> {
	if (HasGenerator(SfHoldModEnv)) {
		SHORT cent = generators[SfHoldModEnv].sh_amount;
		return TimeCentToSeconds(cent);
	} else {
		return std::nullopt;
	}
}

auto ZoneBase::GetDecayModEnv() const -> std::optional<double> {
	if (HasGenerator(SfDecayModEnv)) {
		SHORT cent = generators[SfDecayModEnv].sh_amount;
		return TimeCentToSeconds(cent);
	} else {
		return std::nullopt;
	}
}

auto ZoneBase::GetSustainModEnv() const -> std::optional<int16_t> {
	return PLAIN_S16DATA(SfSustainModEnv);
}

auto ZoneBase::GetReleaseModEnv() const -> std::optional<double> {
	if (HasGenerator(SfReleaseModEnv)) {
		SHORT cent = generators[SfReleaseModEnv].sh_amount;
		return TimeCentToSeconds(cent);
	} else {
		return std::nullopt;
	}
}

auto ZoneBase::GetKeynumToModEnvHold() const -> std::optional<int16_t> {
	return PLAIN_S16DATA(SfKeynumToModEnvHold);
}

auto ZoneBase::GetKeynumToModEnvDecay() const -> std::optional<int16_t> {
	return PLAIN_S16DATA(SfKeynumToModEnvDecay);
}

auto ZoneBase::GetDelayVolEnv() const -> std::optional<double> {
	if (HasGenerator(SfDelayVolEnv)) {
		SHORT cent = generators[SfDelayVolEnv].sh_amount;
		return TimeCentToSeconds(cent);
	} else {
		return std::nullopt;
	}
}

auto ZoneBase::GetAttackVolEnv() const -> std::optional<double> {
	if (HasGenerator(SfAttackVolEnv)) {
		SHORT cent = generators[SfAttackVolEnv].sh_amount;
		return TimeCentToSeconds(cent);
	} else {
		return std::nullopt;
	}
}

auto ZoneBase::GetHoldVolEnv() const -> std::optional<double> {
	if (HasGenerator(SfHoldVolEnv)) {
		SHORT cent = generators[SfHoldVolEnv].sh_amount;
		return TimeCentToSeconds(cent);
	} else {
		return std::nullopt;
	}
}

auto ZoneBase::GetDecayVolEnv() const -> std::optional<double> {
	if (HasGenerator(SfDecayVolEnv)) {
		SHORT cent = generators[SfDecayVolEnv].sh_amount;
		return TimeCentToSeconds(cent);
	} else {
		return std::nullopt;
	}
}

auto ZoneBase::GetSustainVolEnv() const -> std::optional<int16_t> {
	return PLAIN_S16DATA(SfSustainVolEnv);
}

auto ZoneBase::GetReleaseVolEnv() const -> std::optional<double> {
	if (HasGenerator(SfReleaseVolEnv)) {
		SHORT cent = generators[SfReleaseVolEnv].sh_amount;
		return TimeCentToSeconds(cent);
	} else {
		return std::nullopt;
	}
}

auto ZoneBase::GetKeynumToVolEnvHold() const -> std::optional<int16_t> {
	return PLAIN_S16DATA(SfKeynumToVolEnvHold);
}

auto ZoneBase::GetKeynumToVolEnvDecay() const -> std::optional<int16_t> {
	return PLAIN_S16DATA(SfKeynumToVolEnvDecay);
}

auto ZoneBase::GetKeyRange() const -> std::optional<Ranges<uint8_t>> {
	if (HasGenerator(SfKeyRange)) {
		RangesType ranges = generators[SfKeyRange].ranges;
		Ranges<uint8_t> res;
		res.start = ranges.by_lo;
		res.end = ranges.by_hi;
		return res;
	} else {
		return std::nullopt;
	}
}

auto ZoneBase::GetVelRange() const -> std::optional<Ranges<uint8_t>> {
	if (HasGenerator(SfVelRange)) {
		RangesType ranges = generators[SfVelRange].ranges;
		Ranges<uint8_t> res;
		res.start = ranges.by_lo;
		res.end = ranges.by_hi;
		return res;
	} else {
		return std::nullopt;
	}
}

auto ZoneBase::GetInitialAttenuation() const -> std::optional<int16_t> {
	return PLAIN_S16DATA(SfInitialAttenuation);
}

auto ZoneBase::GetCoarseTune() const -> std::optional<int16_t> {
	return PLAIN_S16DATA(SfCoarseTune);
}

auto ZoneBase::GetFineTune() const -> std::optional<int16_t> {
	return PLAIN_S16DATA(SfFineTune);
}

auto ZoneBase::GetScaleTuning() const -> std::optional<int16_t> {
	return PLAIN_S16DATA(SfScaleTuning);
}

#define ZONEBASE_S16_PLAIN_SETTER_IMPL(GeneratorType) \
	void ZoneBase::Set##GeneratorType(std::optional<int16_t> x) { \
		if (x.has_value()) { \
			generators[Sf##GeneratorType].sh_amount = x.value(); \
			active_gens.set(Sf##GeneratorType); \
		} else { \
			active_gens.reset(Sf##GeneratorType); \
		} \
	}

#define ZONEBASE_TIME_CENT_SETTER_IMPL(GeneratorType, MinVal, MaxVal) \
	void ZoneBase::Set##GeneratorType(std::optional<double> x) { \
		if (x.has_value()) { \
			SHORT cent = SecondsToTimeCent(x.value()); \
			if (cent < (MinVal)) { \
				cent = (MinVal); \
			} else if (cent > (MaxVal)) { \
				cent = (MaxVal); \
			} \
			generators[Sf##GeneratorType].sh_amount = cent; \
			active_gens.set(Sf##GeneratorType); \
		} else { \
			active_gens.reset(Sf##GeneratorType); \
		} \
	}

#define ZONEBASE_ABSL_CENT_SETTER_IMPL(GeneratorType, MinVal, MaxVal) \
	void ZoneBase::Set##GeneratorType(std::optional<double> x) { \
		if (x.has_value()) { \
			SHORT cent = HertzToAbsoluteCent(x.value()); \
			if (cent < (MinVal)) { \
				cent = (MinVal); \
			} else if (cent > (MaxVal)) { \
				cent = (MaxVal); \
			} \
			generators[Sf##GeneratorType].sh_amount = cent; \
			active_gens.set(Sf##GeneratorType); \
		} else { \
			active_gens.reset(Sf##GeneratorType); \
		} \
	}

#define ZONEBASE_RANGE_SETTER_IMPL(GeneratorType) \
	void ZoneBase::Set##GeneratorType(std::optional<Ranges<uint8_t>> x) { \
		if (x.has_value()) { \
			generators[Sf##GeneratorType].ranges.by_lo = x->start; \
			generators[Sf##GeneratorType].ranges.by_hi = x->end; \
			active_gens.set(Sf##GeneratorType); \
		} else { \
			active_gens.reset(Sf##GeneratorType); \
		} \
	}

ZONEBASE_S16_PLAIN_SETTER_IMPL(ModLfoToPitch)
ZONEBASE_S16_PLAIN_SETTER_IMPL(VibLfoToPitch)
ZONEBASE_S16_PLAIN_SETTER_IMPL(ModEnvToPitch)
ZONEBASE_ABSL_CENT_SETTER_IMPL(InitialFilterFc, 1500, 13500)
ZONEBASE_S16_PLAIN_SETTER_IMPL(InitialFilterQ)
ZONEBASE_S16_PLAIN_SETTER_IMPL(ModLfoToFilterFc)
ZONEBASE_S16_PLAIN_SETTER_IMPL(ModEnvToFilterFc)
ZONEBASE_S16_PLAIN_SETTER_IMPL(ModLfoToVolume)
ZONEBASE_S16_PLAIN_SETTER_IMPL(ChorusEffectsSend)
ZONEBASE_S16_PLAIN_SETTER_IMPL(ReverbEffectsSend)
ZONEBASE_S16_PLAIN_SETTER_IMPL(Pan)
ZONEBASE_TIME_CENT_SETTER_IMPL(DelayModLFO,   -12000, 5000)
ZONEBASE_ABSL_CENT_SETTER_IMPL(FreqModLFO,     -1600, 4500)
ZONEBASE_TIME_CENT_SETTER_IMPL(DelayVibLFO,   -12000, 5000)
ZONEBASE_ABSL_CENT_SETTER_IMPL(FreqVibLFO,    -16000, 4500)  
ZONEBASE_TIME_CENT_SETTER_IMPL(DelayModEnv,   -12000, 5000)
ZONEBASE_TIME_CENT_SETTER_IMPL(AttackModEnv,  -12000, 8000)
ZONEBASE_TIME_CENT_SETTER_IMPL(HoldModEnv,    -12000, 5000)
ZONEBASE_TIME_CENT_SETTER_IMPL(DecayModEnv,   -12000, 8000)
ZONEBASE_S16_PLAIN_SETTER_IMPL(SustainModEnv)
ZONEBASE_TIME_CENT_SETTER_IMPL(ReleaseModEnv, -12000, 8000)
ZONEBASE_S16_PLAIN_SETTER_IMPL(KeynumToModEnvHold)
ZONEBASE_S16_PLAIN_SETTER_IMPL(KeynumToModEnvDecay)
ZONEBASE_TIME_CENT_SETTER_IMPL(DelayVolEnv,   -12000, 5000)
ZONEBASE_TIME_CENT_SETTER_IMPL(AttackVolEnv,  -12000, 8000)
ZONEBASE_TIME_CENT_SETTER_IMPL(HoldVolEnv,    -12000, 5000)
ZONEBASE_TIME_CENT_SETTER_IMPL(DecayVolEnv,   -12000, 8000)
ZONEBASE_S16_PLAIN_SETTER_IMPL(SustainVolEnv)
ZONEBASE_TIME_CENT_SETTER_IMPL(ReleaseVolEnv, -12000, 8000)
ZONEBASE_S16_PLAIN_SETTER_IMPL(KeynumToVolEnvHold)
ZONEBASE_S16_PLAIN_SETTER_IMPL(KeynumToVolEnvDecay)
ZONEBASE_RANGE_SETTER_IMPL(KeyRange)
ZONEBASE_RANGE_SETTER_IMPL(VelRange)
ZONEBASE_S16_PLAIN_SETTER_IMPL(InitialAttenuation)
ZONEBASE_S16_PLAIN_SETTER_IMPL(CoarseTune)
ZONEBASE_S16_PLAIN_SETTER_IMPL(FineTune)
ZONEBASE_S16_PLAIN_SETTER_IMPL(ScaleTuning)

InstZone::InstZone(const BYTE *gen_list_ptr, DWORD count) : ZoneBase(gen_list_ptr, count) {
	if (HasGenerator(SfSampleID)) {
		sample = SfHandle{generators[SfSampleID].w_amount};
	}
}

DWORD InstZone::RequiredSize() const {
	return active_gens.count() * sizeof(SfInstGenList);
}

SflibError InstZone::SerializeGenerators(BYTE *dst, BYTE **end, const SampleManager& sample_manager) const {
	BYTE* pos = dst;
	DWORD gen_count = 0;
	const auto append_generator = [&pos, &gen_count](SFGenerator type, GenAmountType amt) {
		SfInstGenList bits;
		bits.sf_gen_oper = type;
		bits.gen_amount = amt;
		std::memcpy(pos, &bits, sizeof(bits));
		pos += sizeof(bits);
		gen_count++;
	};

	if (active_gens[SfKeyRange]) {
		append_generator(SfKeyRange, generators[SfKeyRange]);
		if (active_gens[SfVelRange]) {
			append_generator(SfVelRange, generators[SfVelRange]);
		}
	}
	for (WORD gen_type = 0; gen_type < SfEndOper; gen_type++) {
		if (   gen_type == SfKeyRange
			|| gen_type == SfVelRange
			|| gen_type == SfSampleID
			|| !active_gens[gen_type]
		) {
			continue;
		}
		append_generator(static_cast<SFGenerator>(gen_type), generators[gen_type]);
	}
	if (active_gens[SfSampleID]) {
		GenAmountType tmp;
		auto sample_id = sample_manager.GetSampleID(sample.value());
		if (sample_id.has_value() == false) {
			if (end) {
				*end = pos;
			}
			return SFLIB_NO_SUCH_SAMPLE;
		}
		tmp.w_amount = static_cast<WORD>(sample_id.value());
		append_generator(SfSampleID, tmp);
	}

	if (end) {
		*end = pos;
	}

	return SFLIB_SUCCESS;
}

auto InstZone::GetStartAddrsOffset() const -> std::optional<int16_t>
{
	return PLAIN_S16DATA(SfEndAddrsOffset);
}

auto InstZone::GetEndAddrsOffset() const -> std::optional<int16_t>
{
	return PLAIN_S16DATA(SfEndAddrsOffset);
}

auto InstZone::GetStartloopAddrsOffset() const -> std::optional<int16_t> {
	return PLAIN_S16DATA(SfStartloopAddrsOffset);
}

auto InstZone::GetEndloopAddrsOffset() const -> std::optional<int16_t> {
	return PLAIN_S16DATA(SfEndloopAddrsOffset);
}

auto InstZone::GetStartAddrsCoarseOffset() const -> std::optional<int16_t> {
	return PLAIN_S16DATA(SfStartAddrsCoarseOffset);
}

auto InstZone::GetEndAddrsCoarseOffset() const -> std::optional<int16_t> {
	return PLAIN_S16DATA(SfEndAddrsCoarseOffset);
}

auto InstZone::GetStartloopAddrsCoarseOffset() const -> std::optional<int16_t> {
	return PLAIN_S16DATA(SfStartloopAddrsCoarseOffset);
}

auto InstZone::GetEndloopAddrsCoarseOffset() const -> std::optional<int16_t> {
	return PLAIN_S16DATA(SfEndloopAddrsCoarseOffset);
}

auto InstZone::GetKeynum() const -> std::optional<int16_t> {
	return PLAIN_S16DATA(SfKeynum);
}

auto InstZone::GetVelocity() const -> std::optional<int16_t> {
	return PLAIN_S16DATA(SfVelocity);
}

auto InstZone::GetSampleHandle() const -> std::optional<SfHandle> {
	return sample;
}

auto InstZone::GetSampleModes() const -> std::optional<LoopMode> {
	if (HasGenerator(SfSampleModes)) {
		switch (generators[SfSampleModes].w_amount) {
			case 0: return LoopMode::NoLoop;
			case 1: case 2: return LoopMode::Loop;
			case 3: return LoopMode::LoopWithRemainder;
			default: return LoopMode::NoLoop;
		}
	} else {
		return std::nullopt;
	}
}

auto InstZone::GetExclusiveClass() const -> std::optional<int16_t> {
	return PLAIN_S16DATA(SfExclusiveClass);
}

auto InstZone::GetOverridingRootKey() const -> std::optional<int16_t> {
	return PLAIN_S16DATA(SfOverridingRootKey);
}

#define INSTZONE_S16_PLAIN_SETTER_IMPL(GeneratorType) \
	void InstZone::Set##GeneratorType(std::optional<int16_t> x) { \
		if (x.has_value()) { \
			generators[Sf##GeneratorType].sh_amount = x.value(); \
			active_gens.set(Sf##GeneratorType); \
		} else { \
			active_gens.reset(Sf##GeneratorType); \
		} \
	}

INSTZONE_S16_PLAIN_SETTER_IMPL(StartAddrsOffset)
INSTZONE_S16_PLAIN_SETTER_IMPL(EndAddrsOffset)
INSTZONE_S16_PLAIN_SETTER_IMPL(StartloopAddrsOffset)
INSTZONE_S16_PLAIN_SETTER_IMPL(EndloopAddrsOffset)
INSTZONE_S16_PLAIN_SETTER_IMPL(StartAddrsCoarseOffset)
INSTZONE_S16_PLAIN_SETTER_IMPL(EndAddrsCoarseOffset)
INSTZONE_S16_PLAIN_SETTER_IMPL(StartloopAddrsCoarseOffset)
INSTZONE_S16_PLAIN_SETTER_IMPL(EndloopAddrsCoarseOffset)
INSTZONE_S16_PLAIN_SETTER_IMPL(Keynum)
INSTZONE_S16_PLAIN_SETTER_IMPL(Velocity)
INSTZONE_S16_PLAIN_SETTER_IMPL(ExclusiveClass)
INSTZONE_S16_PLAIN_SETTER_IMPL(OverridingRootKey)

void InstZone::SetSampleHandle(std::optional<SfHandle> x) {
	if (x.has_value()) {
		active_gens.set(SfSampleID);
	} else {
		active_gens.reset(SfSampleID);
	}
	sample = x;
}

void InstZone::SetSampleModes(std::optional<LoopMode> x) {
	if (x.has_value()) {
		active_gens.set(SfSampleModes);
		switch (x.value()) {
			case LoopMode::NoLoop:
				generators[SfSampleModes].w_amount = 0;
				break;
			case LoopMode::Loop:
				generators[SfSampleModes].w_amount = 1;
				break;
			case LoopMode::LoopWithRemainder:
				generators[SfSampleModes].w_amount = 3;
				break;
		}
	} else {
		active_gens.reset(SfSampleModes);
	}
}

sflib::PresetZone::PresetZone(const BYTE *gen_list_ptr, DWORD count) : ZoneBase(gen_list_ptr, count) {
	if (HasGenerator(SfInstrument)) {
		instrument = SfHandle{generators[SfInstrument].w_amount};
	}
}

DWORD sflib::PresetZone::RequiredSize() const {
	return active_gens.count() * sizeof(SfGenList);
}

SflibError sflib::PresetZone::SerializeGenerators(BYTE *dst, BYTE **end, const InstrumentManager& inst_manager) const {
	BYTE* pos = dst;
	DWORD gen_count = 0;
	const auto append_generator = [&pos, &gen_count](SFGenerator type, GenAmountType amt) {
		SfGenList bits;
		bits.sf_gen_oper = type;
		bits.gen_amount = amt;
		std::memcpy(pos, &bits, sizeof(bits));
		pos += sizeof(bits);
		gen_count++;
	};

	if (active_gens[SfKeyRange]) {
		append_generator(SfKeyRange, generators[SfKeyRange]);
		if (active_gens[SfVelRange]) {
			append_generator(SfVelRange, generators[SfVelRange]);
		}
	}
	for (WORD gen_type = 0; gen_type < SfEndOper; gen_type++) {
		if (   gen_type == SfKeyRange
			|| gen_type == SfVelRange
			|| gen_type == SfInstrument
			|| !active_gens[gen_type]
		) {
			continue;
		}
		append_generator(static_cast<SFGenerator>(gen_type), generators[gen_type]);
	}
	if (active_gens[SfInstrument]) {
		GenAmountType tmp;

		auto inst_id = inst_manager.GetInstID(instrument.value());
		if (inst_id.has_value() == false) {
			if (end) {
				*end = pos;
			}
			return SFLIB_NO_SUCH_INSTRUMENT;
		}
		tmp.w_amount = static_cast<WORD>(inst_id.value());
		append_generator(SfInstrument, tmp);
	}

	if (end) {
		*end = pos;
	}

	return SFLIB_SUCCESS;
}

auto PresetZone::GetInstrument() const -> std::optional<SfHandle> {
	return instrument;
}

void PresetZone::SetInstrument(std::optional<SfHandle> x) {
	if (x.has_value()) {
		active_gens.set(SfInstrument);
	} else {
		active_gens.reset(SfInstrument);
	}
	instrument = x;
}
