#pragma once

#include "sfspec.hpp"
#include "sfhandle.hpp"
#include <cstdint>
#include <optional>
#include <bitset>
#include <array>
#include <cassert>

namespace sflib {
	class InstrumentManager;

	class SfPresetZone {
		friend class SoundFontImpl;
		PZoneHandle self_handle;
	public:
		SfPresetZone(PZoneHandle handle) : self_handle(handle) {}; // new zone

		PZoneHandle GetHandle() const { return self_handle; }

		DWORD RequiredSize() const;

		bool IsEmpty() const noexcept { return active_gens.count() == 0; }
		DWORD GeneratorCount() const noexcept { return active_gens.count(); }
		bool HasGenerator(SFGenerator type) const {
			assert(static_cast<WORD>(type) < SfGenEndOper);
			return active_gens[static_cast<WORD>(type)];
		}
		bool HasModulator(SFModulator type) const { return false; }

		SfPresetZone& CopyProperties(const SfPresetZone& zone);
		SfPresetZone& MoveProperties(SfPresetZone&& zone);

		SflibError SerializeGenerators(BYTE* dst, BYTE** end, const InstrumentManager& inst_manager) const;

		// @return degree, in cents, to which a full scale excursion of Modulation LFO will influence pitch
		auto GetModLfoToPitch() const -> std::int16_t;
		// @return degree, in cents, to which a full scale excursion of Vibrato LFO will influence pitch
		auto GetVibLfoToPitch() const -> std::int16_t;
		// @return degree, in cents, to which a full scale excursion of Modulation Envelope will influence pitch
		auto GetModEnvToPitch() const -> std::int16_t;
		// @return cutoff/resonant frequency, in Hz, of the lowpass filter
		auto GetInitialFilterFc() const -> double;
		// @return Q value, in centi-bels, of the lowpass filter
		auto GetInitialFilterQ() const  -> std::int16_t;
		// @return degree, in cents, to which a full scale excursion of Modulation LFO will influence filter cutoff frequency
		auto GetModLfoToFilterFc() const -> std::int16_t;
		// @return degree, in cents, to which a full scale excursion of Modulation Envelope will influence filter cutoff frequency
		auto GetModEnvToFilterFc() const -> std::int16_t;
		// @return degree, in centi-bels, to which a full scale excursion of Modulation LFO will influence volume 
		auto GetModLfoToVolume() const -> std::int16_t;
		// @return degree, in 0.1% units, to which the audio output of the note is sent to the chorus effect processor
		auto GetChorusEffectsSend() const -> std::int16_t;
		// @return degree, in 0.1% units, to which the audio output of the note is sent to the reverb effect processor
		auto GetReverbEffectsSend() const -> std::int16_t;
		// @return degree, in 0.1% units, to which the dry audio output of the note is positioned to the left or right output
		auto GetPan() const -> std::int16_t;
		// @return delay time, in secs, of Modulation LFO
		auto GetDelayModLFO() const   -> double;
		// @return frequency, in Hz, of Modulation LFO (triangluar period)
		auto GetFreqModLFO() const    -> double;
		// @return delay time, in secs, of Vibrato LFO
		auto GetDelayVibLFO() const   -> double;
		// @return frequency, in Hz, of Vibrato LFO (triangluar period)
		auto GetFreqVibLFO() const    -> double;
		// @return delay time, in secs, of Modulation Envelope
		auto GetDelayModEnv() const   -> double;
		// @return attack time, in secs, of Modulation Envelope
		auto GetAttackModEnv() const  -> double;
		// @return hold time, in secs, of Modulation Envelope
		auto GetHoldModEnv() const    -> double;
		// @return decay time, in secs, of Modulation Envelope
		auto GetDecayModEnv() const   -> double;
		// @return sustain level, in 0.1% units, of Modulation Envelope
		auto GetSustainModEnv() const -> std::int16_t;
		// @return release time, in secs, of Modulation Envelope
		auto GetReleaseModEnv() const -> double;
		// @return hold time, in tcent/key unit, of Modulation Envelope based on key number
		auto GetKeynumToModEnvHold() const -> std::int16_t;
		// @return decay time, in tcent/key unit, of Modulation Envelope based on key number
		auto GetKeynumToModEnvDecay() const -> std::int16_t;
		// @return delay time, in secs, of Volume Envelope
		auto GetDelayVolEnv() const   -> double;
		// @return attack time, in secs, of Volume Envelope
		auto GetAttackVolEnv() const  -> double;
		// @return hold time, in secs, of Volume Envelope
		auto GetHoldVolEnv() const    -> double;
		// @return decay time, in secs, of Volume Envelope
		auto GetDecayVolEnv() const   -> double;
		// @return sustain level, in centi-bels, of Volume Envelope
		auto GetSustainVolEnv() const -> std::int16_t;
		// @return release time, in secs, of Volume Envelope
		auto GetReleaseVolEnv() const -> double;
		// @return hold time, in tcent/key unit, of Volume Envelope based on key number
		auto GetKeynumToVolEnvHold() const -> std::int16_t;
		// @return decay time, in tcent/key unit, of Volume Envelope based on key number
		auto GetKeynumToVolEnvDecay() const -> std::int16_t;
		// @return key range (minmax MIDI key number ranges in which the zone is active)
		auto GetKeyRange() const -> Ranges<std::uint8_t>;
		// @return vel range (minmax MIDI velocity ranges in which the zone is active)
		auto GetVelRange() const -> Ranges<std::uint8_t>;
		// @return attenuation, in centi-bels, by which a note is attenuated below full scale
		auto GetInitialAttenuation() const -> std::int16_t;
		// @return pitch offset, in semitones
		auto GetCoarseTune() const -> std::int16_t;
		// @return pitch offset, in cents
		auto GetFineTune() const -> std::int16_t;
		// @return degree to which MIDI key number influences pitch
		auto GetScaleTuning() const -> std::int16_t;

		// @param x cents(-12000 ~ 12000 inclusive), by which a full scale excursion of Modulation LFO will influence pitch
		auto SetModLfoToPitch(std::optional<std::int16_t> x) -> SfPresetZone&;
		// @param x cents(-12000 ~ 12000 inclusive), by which a full scale excursion of Vibrato LFO will influence pitch
		auto SetVibLfoToPitch(std::optional<std::int16_t> x) -> SfPresetZone&;
		// @param x cents(-12000 ~ 12000 inclusive), by which a full scale excursion of Modulation Envelope will influence pitch
		auto SetModEnvToPitch(std::optional<std::int16_t> x) -> SfPresetZone&;
		// @param x cutoff/resonant frequency, in Hz(20.0 ~ 20000.0 inclusive), of the lowpass filter(20kHz indicates that the filter will be bypassed)
		auto SetInitialFilterFc(std::optional<double> x) -> SfPresetZone&;
		// @param x Q value, in centi-bels(0* ~ 960 inclusive), of the lowpass filter
		auto SetInitialFilterQ(std::optional<std::int16_t> x) -> SfPresetZone&;
		// @param x cents(-12000 ~ 12000 inclusive), by which a full scale excursion of Modulation LFO will influence filter cutoff frequency
		auto SetModLfoToFilterFc(std::optional<std::int16_t> x) -> SfPresetZone&;
		// @param x cents(-12000 ~ 12000 inclusive), by which a full scale excursion of Modulation Envelope will influence filter cutoff frequency
		auto SetModEnvToFilterFc(std::optional<std::int16_t> x) -> SfPresetZone&;
		// @param x centi-bels(-960 ~ 960 inclusive), by which a full scale excursion of Modulation LFO will influence volume 
		auto SetModLfoToVolume(std::optional<std::int16_t> x) -> SfPresetZone&;
		// @param x 0.1% units(0* ~ 1000 inclusive), by which the audio output of the note is sent to the chorus effect processor
		auto SetChorusEffectsSend(std::optional<std::int16_t> x) -> SfPresetZone&;
		// @param x 0.1% units(0* ~ 1000 inclusive), by which the audio output of the note is sent to the reverb effect processor
		auto SetReverbEffectsSend(std::optional<std::int16_t> x) -> SfPresetZone&;
		// @param x 0.1% units(-500 ~ 500 inclusive), by which the dry audio output of the note is positioned to the left or right output
		auto SetPan(std::optional<std::int16_t> x) -> SfPresetZone&;
		// @param x delay time, in secs(0.001 ~ 20.0), of Modulation LFO
		auto SetDelayModLFO(std::optional<double> x) -> SfPresetZone&;
		// @param x frequency, in Hz(0.001 ~ 100.0), of Modulation LFO (triangluar period)
		auto SetFreqModLFO(std::optional<double> x) -> SfPresetZone&;
		// @param x delay time, in secs(0.001 ~ 20.0), of Vibrato LFO
		auto SetDelayVibLFO(std::optional<double> x) -> SfPresetZone&;
		// @param x frequency, in Hz(0.001 ~ 100.0), of Vibrato LFO (triangluar period)
		auto SetFreqVibLFO(std::optional<double> x) -> SfPresetZone&;
		// @param x delay time, in secs(0.001 ~ 20.0), of Modulation Envelope
		auto SetDelayModEnv(std::optional<double> x) -> SfPresetZone&;
		// @param x attack time, in secs(0.001 ~ 100.0), of Modulation Envelope
		auto SetAttackModEnv(std::optional<double> x) -> SfPresetZone&;
		// @param x hold time, in secs(0.001 ~ 20.0), of Modulation Envelope
		auto SetHoldModEnv(std::optional<double> x) -> SfPresetZone&;
		// @param x decay time, in secs(0.001 ~ 100.0), of Modulation Envelope
		auto SetDecayModEnv(std::optional<double> x) -> SfPresetZone&;
		// @param x sustain level, in 0.1% units(0* ~ 1000*), of Modulation Envelope
		auto SetSustainModEnv(std::optional<std::int16_t> x) -> SfPresetZone&;
		// @param x release time, in secs(0.001 ~ 100.0), of Modulation Envelope
		auto SetReleaseModEnv(std::optional<double> x) -> SfPresetZone&;
		// @param x hold time, in tcent/key unit(-1200 ~ 1200), of Modulation Envelope based on key number
		auto SetKeynumToModEnvHold(std::optional<std::int16_t> x) -> SfPresetZone&;
		// @param x decay time, in tcent/key unit(-1200 ~ 1200), of Modulation Envelope based on key number
		auto SetKeynumToModEnvDecay(std::optional<std::int16_t> x) -> SfPresetZone&;
		// @param x delay time, in secs(0.001 ~ 20.0), of Volume Envelope
		auto SetDelayVolEnv(std::optional<double> x) -> SfPresetZone&;
		// @param x attack time, in secs(0.001 ~ 100.0), of Volume Envelope
		auto SetAttackVolEnv(std::optional<double> x) -> SfPresetZone&;
		// @param x hold time, in secs(0.001 ~ 20.0), of Volume Envelope
		auto SetHoldVolEnv(std::optional<double> x) -> SfPresetZone&;
		// @param x decay time, in secs(0.001 ~ 100.0), of Volume Envelope
		auto SetDecayVolEnv(std::optional<double> x) -> SfPresetZone&;
		// @param x sustain level, in centi-bels(0* ~ 1440), of Volume Envelope
		auto SetSustainVolEnv(std::optional<std::int16_t> x) -> SfPresetZone&;
		// @param x release time, in secs(0.001 ~ 100.0), of Volume Envelope
		auto SetReleaseVolEnv(std::optional<double> x) -> SfPresetZone&;
		// @param x hold time, in tcent/key unit(-1200 ~ 1200), of Volume Envelope based on key number
		auto SetKeynumToVolEnvHold(std::optional<std::int16_t> x) -> SfPresetZone&;
		// @param x decay time, in tcent/key unit(-1200 ~ 1200), of Volume Envelope based on key number
		auto SetKeynumToVolEnvDecay(std::optional<std::int16_t> x) -> SfPresetZone&;
		// @param x key range(0-127) (minmax MIDI key number ranges in which the zone is active)
		auto SetKeyRange(std::optional<Ranges<std::uint8_t>> x) -> SfPresetZone&;
		// @param x vel range(0-127) (minmax MIDI velocity ranges in which the zone is active)
		auto SetVelRange(std::optional<Ranges<std::uint8_t>> x) -> SfPresetZone&;
		// @param x attenuation, in centi-bels(0* ~ 1440), by which a note is attenuated below full scale
		auto SetInitialAttenuation(std::optional<std::int16_t> x) -> SfPresetZone&;
		// @param x pitch offset, in semitones(-120 ~ 120)
		auto SetCoarseTune(std::optional<std::int16_t> x) -> SfPresetZone&;
		// @param x pitch offset, in cents(-99 ~ 99)
		auto SetFineTune(std::optional<std::int16_t> x) -> SfPresetZone&;
		// @param x degree(0 ~ 1200) to which MIDI key number influences pitch
		auto SetScaleTuning(std::optional<std::int16_t> x) -> SfPresetZone&;

		//
		// (below properties are Preset Zone Exclusives!)
		//

		auto GetInstrument() const -> std::optional<InstHandle>;
		auto SetInstrument(std::optional<InstHandle> x) -> SfPresetZone&;

	private:
		// Modulators are currently not defined...
		std::bitset<SfGenEndOper> active_gens {};
		std::array<spec::GenAmountType, SfGenEndOper> generators;
		std::optional<InstHandle> instrument;
	};
}