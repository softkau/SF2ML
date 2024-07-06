#ifndef SF2ML_SFMODULATOR_HPP_
#define SF2ML_SFMODULATOR_HPP_

#include "sfhandle.hpp"
#include "sfspec.hpp"
#include <memory>
#include <variant>

namespace SF2ML {
	class SfModulator {
	public:
		SfModulator(ModHandle handle);
		~SfModulator();
		ModHandle GetHandle() const;
		SfModulator(const SfModulator&);
		SfModulator(SfModulator&&) noexcept;
		SfModulator& operator=(const SfModulator&);
		SfModulator& operator=(SfModulator&&) noexcept;

		auto SetSource(SFModulator bits) -> SfModulator&;
		
		auto SetSource(GeneralController controller,
					   bool polarity,
					   bool direction,
					   SfModSourceType shape) -> SfModulator&;

		auto SetSource(MidiController controller,
					   bool polarity,
					   bool direction,
					   SfModSourceType shape) -> SfModulator&;
		
		auto SetAmtSource(SFModulator bits) -> SfModulator&;

		auto SetAmtSource(GeneralController controller,
						  bool polarity,
						  bool direction,
						  SfModSourceType shape) -> SfModulator&;

		auto SetAmtSource(MidiController controller,
						  bool polarity,
						  bool direction,
						  SfModSourceType shape) -> SfModulator&;

		auto SetTransform(SFTransform transform) -> SfModulator&;

		auto SetDestination(SFGenerator dest) -> SfModulator&;
		
		auto SetDestination(ModHandle dest) -> SfModulator&;

		auto GetSourceController() const -> std::variant<GeneralController, MidiController>;
		auto GetAmtSourceController() const -> std::variant<GeneralController, MidiController>;
		auto GetSourcePolarity() const -> bool;
		auto GetSourceDirection() const -> bool;
		auto GetSourceShape() const -> SfModSourceType;
		auto GetAmtSourcePolarity() const -> bool;
		auto GetAmtSourceDirection() const -> bool;
		auto GetAmtSourceShape() const -> SfModSourceType;

		auto GetTransform() const -> SFTransform;
		auto GetDestination() const -> std::variant<SFGenerator, ModHandle>;
	private:
		std::unique_ptr<class SfModulatorImpl> pimpl;
	};
}

#endif