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
		SHORT mod_amt = 0;
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

SfModulator::SfModulator(const SfModulator& rhs) {
	pimpl = std::make_unique<SfModulatorImpl>(*rhs.pimpl);
}

SfModulator::SfModulator(SfModulator&& rhs) noexcept {
	pimpl = std::move(rhs.pimpl);
}

SfModulator& SF2ML::SfModulator::operator=(const SfModulator& rhs) {
	pimpl = std::make_unique<SfModulatorImpl>(*rhs.pimpl);
	return *this;
}

SfModulator& SfModulator::operator=(SfModulator&& rhs) noexcept {
	pimpl = std::move(rhs.pimpl);
	return *this;
}

auto SfModulator::SetSource(SFModulator bits) -> SfModulator& {
	pimpl->src = bits;
	return *this;
}

auto SfModulator::SetSource(GeneralController controller,
							bool polarity,
							bool direction,
							SfModSourceType shape) -> SfModulator& {
	pimpl->src = 0;
	pimpl->src |= (static_cast<BYTE>(controller) & 0b0111'1111); // index
	pimpl->src |= 1 << 7; // cc
	pimpl->src |= direction << 8; // d
	pimpl->src |= polarity << 9; // p
	pimpl->src |= (static_cast<BYTE>(shape) & 0b0011'1111) << 10; // type
	
	return *this;
}

auto SfModulator::SetSource(MidiController controller,
							bool polarity,
							bool direction,
							SfModSourceType shape) -> SfModulator& {
	pimpl->src = 0;
	pimpl->src |= (static_cast<BYTE>(controller) & 0b0111'1111); // index
	pimpl->src |= 0 << 7; // cc
	pimpl->src |= direction << 8; // d
	pimpl->src |= polarity << 9; // p
	pimpl->src |= (static_cast<BYTE>(shape) & 0b0011'1111) << 10; // type
	
	return *this;
}

auto SfModulator::SetAmtSource(SFModulator bits) -> SfModulator& {
	pimpl->amt_src = bits;
	return *this;
}

auto SfModulator::SetAmtSource(GeneralController controller,
							   bool polarity,
							   bool direction,
							   SfModSourceType shape) -> SfModulator& {
	if (controller == GeneralController::Link) {
		return *this;
	}

	pimpl->amt_src = 0;
	pimpl->amt_src |= (static_cast<BYTE>(controller) & 0b0111'1111); // index
	pimpl->amt_src |= 1 << 7; // cc
	pimpl->amt_src |= direction << 8; // d
	pimpl->amt_src |= polarity << 9; // p
	pimpl->amt_src |= (static_cast<BYTE>(shape) & 0b0011'1111) << 10; // type

	return *this;
}

auto SfModulator::SetAmtSource(MidiController controller,
							   bool polarity,
							   bool direction,
							   SfModSourceType shape) -> SfModulator& {
	pimpl->amt_src = 0;
	pimpl->amt_src |= (static_cast<BYTE>(controller) & 0b0111'1111); // index
	pimpl->amt_src |= 0 << 7; // cc
	pimpl->amt_src |= direction << 8; // d
	pimpl->amt_src |= polarity << 9; // p
	pimpl->amt_src |= (static_cast<BYTE>(shape) & 0b0011'1111) << 10; // type
	
	return *this;
}

auto SfModulator::SetTransform(SFTransform transform) -> SfModulator& {
	pimpl->trans = transform;
	return *this;
}

auto SfModulator::SetDestination(SFGenerator dest) -> SfModulator& {
	pimpl->dst = static_cast<SFGenerator>(dest & 0x7FFF);
	return *this;
}

auto SfModulator::SetDestination(ModHandle dest) -> SfModulator& {
	pimpl->dst = static_cast<SFGenerator>(dest.value | 0x8000);
	return *this;
}

auto SfModulator::SetModAmount(std::int16_t amt) -> SfModulator& {
	pimpl->mod_amt = amt;
	return *this;
}

auto SfModulator::GetSourceController() const -> std::variant<GeneralController, MidiController> {
	if (pimpl->src & 0x80) {
		return static_cast<MidiController>(pimpl->src & 0x7F);
	} else {
		return static_cast<GeneralController>(pimpl->src);
	}
}

auto SfModulator::GetAmtSourceController() const -> std::variant<GeneralController, MidiController> {
	if (pimpl->amt_src & 0x80) {
		return static_cast<MidiController>(pimpl->amt_src & 0x7F);
	} else {
		return static_cast<GeneralController>(pimpl->amt_src);
	}
}

auto SfModulator::GetSourcePolarity() const -> bool {
	return pimpl->src & 0x0200;
}

auto SfModulator::GetSourceDirection() const -> bool {
	return pimpl->src & 0x0100;
}

auto SfModulator::GetSourceShape() const -> SfModSourceType {
	return static_cast<SfModSourceType>((pimpl->src >> 10) & 0b0011'1111);
}

auto SfModulator::GetAmtSourcePolarity() const -> bool {
	return pimpl->amt_src & 0x0200;
}

auto SfModulator::GetAmtSourceDirection() const -> bool {
	return pimpl->amt_src & 0x0100;
}

auto SfModulator::GetAmtSourceShape() const -> SfModSourceType {
	return static_cast<SfModSourceType>((pimpl->amt_src >> 10) & 0b0011'1111);
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

auto SfModulator::GetModAmount() const -> std::int16_t {
	return pimpl->mod_amt;
}
