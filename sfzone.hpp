#pragma once

#include "sfspec.hpp"
#include "sfhandle.hpp"
#include <bitset>
#include <array>
#include <optional>
#include <variant>

namespace sflib {
	class SampleManager;
	class InstrumentManager;

	class ZoneBase {
	public:
		ZoneBase() {}; // new zone
		ZoneBase(const BYTE* gen_list_ptr, DWORD count);

		virtual DWORD RequiredSize() const = 0;

		bool IsEmpty() const { return active_gens.count() == 0; }
		DWORD GeneratorCount() const { return active_gens.count(); }
		bool HasGenerator(SFGenerator type) const {
			assert(static_cast<WORD>(type) < 64);
			return active_gens[static_cast<WORD>(type)];
		}
		bool HasModulator(SFModulator type) const { return false; }

		// @return degree, in cents, to which a full scale excursion of Modulation LFO will influence pitch
		auto GetModLfoToPitch() const -> std::optional<int16_t>;
		// @return degree, in cents, to which a full scale excursion of Vibrato LFO will influence pitch
		auto GetVibLfoToPitch() const -> std::optional<int16_t>;
		// @return degree, in cents, to which a full scale excursion of Modulation Envelope will influence pitch
		auto GetModEnvToPitch() const -> std::optional<int16_t>;
		// @return cutoff/resonant frequency, in Hz, of the lowpass filter
		auto GetInitialFilterFc() const -> std::optional<double>;
		// @return Q value, in centi-bels, of the lowpass filter
		auto GetInitialFilterQ() const  -> std::optional<int16_t>;
		// @return degree, in cents, to which a full scale excursion of Modulation LFO will influence filter cutoff frequency
		auto GetModLfoToFilterFc() const -> std::optional<int16_t>;
		// @return degree, in cents, to which a full scale excursion of Modulation Envelope will influence filter cutoff frequency
		auto GetModEnvToFilterFc() const -> std::optional<int16_t>;
		// @return degree, in centi-bels, to which a full scale excursion of Modulation LFO will influence volume 
		auto GetModLfoToVolume() const -> std::optional<int16_t>;
		// @return degree, in 0.1% units, to which the audio output of the note is sent to the chorus effect processor
		auto GetChorusEffectsSend() const -> std::optional<int16_t>;
		// @return degree, in 0.1% units, to which the audio output of the note is sent to the reverb effect processor
		auto GetReverbEffectsSend() const -> std::optional<int16_t>;
		// @return degree, in 0.1% units, to which the dry audio output of the note is positioned to the left or right output
		auto GetPan() const -> std::optional<int16_t>;
		// @return delay time, in secs, of Modulation LFO
		auto GetDelayModLFO() const   -> std::optional<double>;
		// @return frequency, in Hz, of Modulation LFO (triangluar period)
		auto GetFreqModLFO() const    -> std::optional<double>;
		// @return delay time, in secs, of Vibrato LFO
		auto GetDelayVibLFO() const   -> std::optional<double>;
		// @return frequency, in Hz, of Vibrato LFO (triangluar period)
		auto GetFreqVibLFO() const    -> std::optional<double>;
		// @return delay time, in secs, of Modulation Envelope
		auto GetDelayModEnv() const   -> std::optional<double>;
		// @return attack time, in secs, of Modulation Envelope
		auto GetAttackModEnv() const  -> std::optional<double>;
		// @return hold time, in secs, of Modulation Envelope
		auto GetHoldModEnv() const    -> std::optional<double>;
		// @return decay time, in secs, of Modulation Envelope
		auto GetDecayModEnv() const   -> std::optional<double>;
		// @return sustain level, in 0.1% units, of Modulation Envelope
		auto GetSustainModEnv() const -> std::optional<int16_t>;
		// @return release time, in secs, of Modulation Envelope
		auto GetReleaseModEnv() const -> std::optional<double>;
		// @return hold time, in tcent/key unit, of Modulation Envelope based on key number
		auto GetKeynumToModEnvHold() const -> std::optional<int16_t>;
		// @return decay time, in tcent/key unit, of Modulation Envelope based on key number
		auto GetKeynumToModEnvDecay() const -> std::optional<int16_t>;
		// @return delay time, in secs, of Volume Envelope
		auto GetDelayVolEnv() const   -> std::optional<double>;
		// @return attack time, in secs, of Volume Envelope
		auto GetAttackVolEnv() const  -> std::optional<double>;
		// @return hold time, in secs, of Volume Envelope
		auto GetHoldVolEnv() const    -> std::optional<double>;
		// @return decay time, in secs, of Volume Envelope
		auto GetDecayVolEnv() const   -> std::optional<double>;
		// @return sustain level, in centi-bels, of Volume Envelope
		auto GetSustainVolEnv() const -> std::optional<int16_t>;
		// @return release time, in secs, of Volume Envelope
		auto GetReleaseVolEnv() const -> std::optional<double>;
		// @return hold time, in tcent/key unit, of Volume Envelope based on key number
		auto GetKeynumToVolEnvHold() const -> std::optional<int16_t>;
		// @return decay time, in tcent/key unit, of Volume Envelope based on key number
		auto GetKeynumToVolEnvDecay() const -> std::optional<int16_t>;
		// @return key range (minmax MIDI key number ranges in which the zone is active)
		auto GetKeyRange() const -> std::optional<Ranges<uint8_t>>;
		// @return vel range (minmax MIDI velocity ranges in which the zone is active)
		auto GetVelRange() const -> std::optional<Ranges<uint8_t>>;
		// @return attenuation, in centi-bels, by which a note is attenuated below full scale
		auto GetInitialAttenuation() const -> std::optional<int16_t>;
		// @return pitch offset, in semitones
		auto GetCoarseTune() const -> std::optional<int16_t>;
		// @return pitch offset, in cents
		auto GetFineTune() const -> std::optional<int16_t>;
		// @return degree to which MIDI key number influences pitch
		auto GetScaleTuning() const -> std::optional<int16_t>;

		// @param x cents(-12000 ~ 12000 inclusive), by which a full scale excursion of Modulation LFO will influence pitch
		void SetModLfoToPitch(std::optional<int16_t> x);
		// @param x cents(-12000 ~ 12000 inclusive), by which a full scale excursion of Vibrato LFO will influence pitch
		void SetVibLfoToPitch(std::optional<int16_t> x);
		// @param x cents(-12000 ~ 12000 inclusive), by which a full scale excursion of Modulation Envelope will influence pitch
		void SetModEnvToPitch(std::optional<int16_t> x);
		// @param x cutoff/resonant frequency, in Hz(20.0 ~ 20000.0 inclusive), of the lowpass filter(20kHz indicates that the filter will be bypassed)
		void SetInitialFilterFc(std::optional<double> x);
		// @param x Q value, in centi-bels(0* ~ 960 inclusive), of the lowpass filter
		void SetInitialFilterQ(std::optional<int16_t> x);
		// @param x cents(-12000 ~ 12000 inclusive), by which a full scale excursion of Modulation LFO will influence filter cutoff frequency
		void SetModLfoToFilterFc(std::optional<int16_t> x);
		// @param x cents(-12000 ~ 12000 inclusive), by which a full scale excursion of Modulation Envelope will influence filter cutoff frequency
		void SetModEnvToFilterFc(std::optional<int16_t> x);
		// @param x centi-bels(-960 ~ 960 inclusive), by which a full scale excursion of Modulation LFO will influence volume 
		void SetModLfoToVolume(std::optional<int16_t> x);
		// @param x 0.1% units(0* ~ 1000 inclusive), by which the audio output of the note is sent to the chorus effect processor
		void SetChorusEffectsSend(std::optional<int16_t> x);
		// @param x 0.1% units(0* ~ 1000 inclusive), by which the audio output of the note is sent to the reverb effect processor
		void SetReverbEffectsSend(std::optional<int16_t> x);
		// @param x 0.1% units(-500 ~ 500 inclusive), by which the dry audio output of the note is positioned to the left or right output
		void SetPan(std::optional<int16_t> x);
		// @param x delay time, in secs(0.001 ~ 20.0), of Modulation LFO
		void SetDelayModLFO(std::optional<double> x);
		// @param x frequency, in Hz(0.001 ~ 100.0), of Modulation LFO (triangluar period)
		void SetFreqModLFO(std::optional<double> x);
		// @param x delay time, in secs(0.001 ~ 20.0), of Vibrato LFO
		void SetDelayVibLFO(std::optional<double> x);
		// @param x frequency, in Hz(0.001 ~ 100.0), of Vibrato LFO (triangluar period)
		void SetFreqVibLFO(std::optional<double> x);
		// @param x delay time, in secs(0.001 ~ 20.0), of Modulation Envelope
		void SetDelayModEnv(std::optional<double> x);
		// @param x attack time, in secs(0.001 ~ 100.0), of Modulation Envelope
		void SetAttackModEnv(std::optional<double> x);
		// @param x hold time, in secs(0.001 ~ 20.0), of Modulation Envelope
		void SetHoldModEnv(std::optional<double> x);
		// @param x decay time, in secs(0.001 ~ 100.0), of Modulation Envelope
		void SetDecayModEnv(std::optional<double> x);
		// @param x sustain level, in 0.1% units(0* ~ 1000*), of Modulation Envelope
		void SetSustainModEnv(std::optional<int16_t> x);
		// @param x release time, in secs(0.001 ~ 100.0), of Modulation Envelope
		void SetReleaseModEnv(std::optional<double> x);
		// @param x hold time, in tcent/key unit(-1200 ~ 1200), of Modulation Envelope based on key number
		void SetKeynumToModEnvHold(std::optional<int16_t> x);
		// @param x decay time, in tcent/key unit(-1200 ~ 1200), of Modulation Envelope based on key number
		void SetKeynumToModEnvDecay(std::optional<int16_t> x);
		// @param x delay time, in secs(0.001 ~ 20.0), of Volume Envelope
		void SetDelayVolEnv(std::optional<double> x);
		// @param x attack time, in secs(0.001 ~ 100.0), of Volume Envelope
		void SetAttackVolEnv(std::optional<double> x);
		// @param x hold time, in secs(0.001 ~ 20.0), of Volume Envelope
		void SetHoldVolEnv(std::optional<double> x);
		// @param x decay time, in secs(0.001 ~ 100.0), of Volume Envelope
		void SetDecayVolEnv(std::optional<double> x);
		// @param x sustain level, in centi-bels(0* ~ 1440), of Volume Envelope
		void SetSustainVolEnv(std::optional<int16_t> x);
		// @param x release time, in secs(0.001 ~ 100.0), of Volume Envelope
		void SetReleaseVolEnv(std::optional<double> x);
		// @param x hold time, in tcent/key unit(-1200 ~ 1200), of Volume Envelope based on key number
		void SetKeynumToVolEnvHold(std::optional<int16_t> x);
		// @param x decay time, in tcent/key unit(-1200 ~ 1200), of Volume Envelope based on key number
		void SetKeynumToVolEnvDecay(std::optional<int16_t> x);
		// @param x key range(0-127) (minmax MIDI key number ranges in which the zone is active)
		void SetKeyRange(std::optional<Ranges<uint8_t>> x);
		// @param x vel range(0-127) (minmax MIDI velocity ranges in which the zone is active)
		void SetVelRange(std::optional<Ranges<uint8_t>> x);
		// @param x attenuation, in centi-bels(0* ~ 1440), by which a note is attenuated below full scale
		void SetInitialAttenuation(std::optional<int16_t> x);
		// @param x pitch offset, in semitones(-120 ~ 120)
		void SetCoarseTune(std::optional<int16_t> x);
		// @param x pitch offset, in cents(-99 ~ 99)
		void SetFineTune(std::optional<int16_t> x);
		// @param x degree(0 ~ 1200) to which MIDI key number influences pitch
		void SetScaleTuning(std::optional<int16_t> x);

	protected:
		// Modulators are currently not defined...
		std::bitset<SfEndOper> active_gens {};
		std::array<GenAmountType, SfEndOper> generators;
	};

	enum class LoopMode {
		NoLoop, Loop, LoopWithRemainder
	};

	class InstZone : public ZoneBase {
	public:
		InstZone() : ZoneBase() {}
		InstZone(const BYTE* gen_list_ptr, DWORD count);

		DWORD RequiredSize() const override;
		SflibError SerializeGenerators(BYTE* dst, BYTE** end, const SampleManager& sample_manager) const;

		// @return offset, in sample data points
		auto GetStartAddrsOffset() const       -> std::optional<int16_t>; 
		// @return offset, in sample data points
		auto GetEndAddrsOffset() const         -> std::optional<int16_t>;
		// @return offset, in sample data points
		auto GetStartloopAddrsOffset() const   -> std::optional<int16_t>;
		// @return offset, in sample data points
		auto GetEndloopAddrsOffset() const     -> std::optional<int16_t>;
		// @return offset, in 32768 sample data points
		auto GetStartAddrsCoarseOffset() const     -> std::optional<int16_t>;
		// @return offset, in 32768 sample data points
		auto GetEndAddrsCoarseOffset() const       -> std::optional<int16_t>;
		// @return offset, in 32768 sample data points
		auto GetStartloopAddrsCoarseOffset() const -> std::optional<int16_t>;
		// @return offset, in 32768 sample data points
		auto GetEndloopAddrsCoarseOffset() const   -> std::optional<int16_t>;
		// @return overriden MIDI key number
		auto GetKeynum() const -> std::optional<int16_t>;
		// @return overriden MIDI velocity value
		auto GetVelocity() const -> std::optional<int16_t>;
		// @return handle of sample to which the zone is linked
		auto GetSampleHandle() const -> std::optional<SfHandle>;
		// @return loop mode with which the zone plays the sample
		auto GetSampleModes() const -> std::optional<LoopMode>;
		// @return exclusive class id; only one instrument can be played for one exclusive class(excpet zero class) (scope: its presetl)
		auto GetExclusiveClass() const -> std::optional<int16_t>;
		// @return MIDI key number of overriding rootkey
		auto GetOverridingRootKey() const -> std::optional<int16_t>;

		// @param x offset, in sample data points
		void SetStartAddrsOffset(std::optional<int16_t> x); 
		// @param x offset, in sample data points
		void SetEndAddrsOffset(std::optional<int16_t> x);
		// @param x offset, in sample data points
		void SetStartloopAddrsOffset(std::optional<int16_t> x);
		// @param x offset, in sample data points
		void SetEndloopAddrsOffset(std::optional<int16_t> x);
		// @param x offset, in 32768 sample data points
		void SetStartAddrsCoarseOffset(std::optional<int16_t> x);
		// @param x offset, in 32768 sample data points
		void SetEndAddrsCoarseOffset(std::optional<int16_t> x);
		// @param x offset, in 32768 sample data points
		void SetStartloopAddrsCoarseOffset(std::optional<int16_t> x);
		// @param x offset, in 32768 sample data points
		void SetEndloopAddrsCoarseOffset(std::optional<int16_t> x);
		// @param x overriden MIDI key number
		void SetKeynum(std::optional<int16_t> x);
		// @param x overriden MIDI velocity value
		void SetVelocity(std::optional<int16_t> x);
		// @param x handle of sample to which the zone is linked
		void SetSampleHandle(std::optional<SfHandle> x);
		// @param x loop mode with which the zone plays the sample
		void SetSampleModes(std::optional<LoopMode> x);
		// @param x exclusive class id; only one instrument can be played for one exclusive class(excpet zero class) (scope: its presetl)
		void SetExclusiveClass(std::optional<int16_t>);
		// @param x MIDI key number of overriding rootkey
		void SetOverridingRootKey(std::optional<int16_t> x);
	private:
		std::optional<SfHandle> sample;
	};

	class PresetZone : public ZoneBase {
	public:
		PresetZone() : ZoneBase() {}
		PresetZone(const BYTE* gen_list_ptr, DWORD count);

		DWORD RequiredSize() const override;
		SflibError SerializeGenerators(BYTE* dst, BYTE** end, const InstrumentManager& inst_manager) const;

		auto GetInstrument() const -> std::optional<SfHandle>;
		void SetInstrument(std::optional<SfHandle> x);
	private:
		std::optional<SfHandle> instrument;
	};
}

