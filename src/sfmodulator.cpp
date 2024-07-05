#include <sfmodulator.hpp>

using namespace SF2ML;

namespace SF2ML {

	class SfModulatorImpl {
	public:
		friend SfModulator;
		SfModulatorImpl(ModHandle handle) : self_handle{handle} {}
	private:
		ModHandle self_handle;

		SFModulator src = 0;
		SFModulator amt_src = 0;
		SFTransform trans = SFTransform::SfModLinearTransform;
		SFGenerator dst = static_cast<SFGenerator>(0);
	};
}

SfModulator::SfModulator(ModHandle handle) {
	pimpl = std::make_unique<SfModulatorImpl>(handle);
}

SfModulator::~SfModulator() {

}

ModHandle SfModulator::GetHandle() const {
	return pimpl->self_handle;
}

auto SfModulator::SetSource(GeneralController controller,
							bool polarity,
							bool direction,
							SfModSourceType shape) -> SfModulator& {
	SFModulatorBitField bits;
	bits.type = static_cast<BYTE>(shape);
	bits.p = polarity;
	bits.d = direction;
	bits.cc = 0;
	bits.index = static_cast<BYTE>(controller);
	std::memcpy(&pimpl->src, &bits, sizeof(bits));

	return *this;
}

auto SfModulator::SetSource(MidiController controller,
							bool polarity,
							bool direction,
							SfModSourceType shape) -> SfModulator& {
	SFModulatorBitField bits;
	bits.type = static_cast<BYTE>(shape);
	bits.p = polarity;
	bits.d = direction;
	bits.cc = 1;
	bits.index = controller;
	std::memcpy(&pimpl->src, &bits, sizeof(bits));
	
	return *this;
}

auto SfModulator::SetAmtSource(GeneralController controller,
							   bool polarity,
							   bool direction,
							   SfModSourceType shape) -> SfModulator& {
	if (controller == GeneralController::Link) {
		return *this;
	}

	SFModulatorBitField bits;
	bits.type = static_cast<BYTE>(shape);
	bits.p = polarity;
	bits.d = direction;
	bits.cc = 0;
	bits.index = static_cast<BYTE>(controller);
	std::memcpy(&pimpl->amt_src, &bits, sizeof(bits));

	return *this;
}

auto SfModulator::SetAmtSource(MidiController controller,
							   bool polarity,
							   bool direction,
							   SfModSourceType shape) -> SfModulator& {
	SFModulatorBitField bits;
	bits.type = static_cast<BYTE>(shape);
	bits.p = polarity;
	bits.d = direction;
	bits.cc = 1;
	bits.index = controller;
	std::memcpy(&pimpl->amt_src, &bits, sizeof(bits));
	
	return *this;
}

auto SfModulator::SetTransform(SFTransform transform) -> SfModulator& {
	pimpl->trans = transform;
	return *this;
}

auto SfModulator::SetDestination(SFGenerator dest) -> SfModulator& {
	pimpl->dst = static_cast<SFGenerator>(dest & 0x7FFF);
}

auto SfModulator::SetDestination(ModHandle dest) -> SfModulator& {
	pimpl->dst = static_cast<SFGenerator>(dest.value | 0x8000);
}

auto SfModulator::GetSourceController() const -> std::variant<GeneralController, MidiController> {
	SFModulatorBitField bits;
	std::memcpy(&bits, &pimpl->src, sizeof(bits));

	if (bits.cc) {
		return static_cast<MidiController>(bits.index);
	} else {
		return static_cast<GeneralController>(bits.index);
	}
}

auto SfModulator::GetAmtSourceController() const -> std::variant<GeneralController, MidiController> {
	SFModulatorBitField bits;
	std::memcpy(&bits, &pimpl->amt_src, sizeof(bits));

	if (bits.cc) {
		return static_cast<MidiController>(bits.index);
	} else {
		return static_cast<GeneralController>(bits.index);
	}
}

auto SfModulator::GetSourcePolarity() const -> bool {
	SFModulatorBitField bits;
	std::memcpy(&bits, &pimpl->src, sizeof(bits));

	return bits.p;
}

auto SfModulator::GetSourceDirection() const -> bool {
	SFModulatorBitField bits;
	std::memcpy(&bits, &pimpl->src, sizeof(bits));

	return bits.d;
}

auto SfModulator::GetAmtSourcePolarity() const -> bool {
	SFModulatorBitField bits;
	std::memcpy(&bits, &pimpl->amt_src, sizeof(bits));

	return bits.p;
}

auto SfModulator::GetAmtSourceDirection() const -> bool {
	SFModulatorBitField bits;
	std::memcpy(&bits, &pimpl->amt_src, sizeof(bits));

	return bits.d;
}

auto SfModulator::GetTransform() const -> SFTransform {
	return pimpl->trans;
}

auto SfModulator::GetDestination() const -> std::variant<SFGenerator, ModHandle> {
	if (pimpl->dst & 0x8000) {
		return ModHandle(pimpl->dst & 0x7FFF);
	} else {
		return pimpl->dst;
	}
}
