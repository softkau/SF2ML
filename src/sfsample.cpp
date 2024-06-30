#include <sfsample.hpp>

#include <cassert>

using namespace SF2ML;

namespace SF2ML {
	class SfSampleImpl {
		friend SfSample;

		const SmplHandle self_handle;
		const SampleBitDepth sample_bit_depth;

		char sample_name[21] {};
		std::vector<BYTE> wav_data {};
		DWORD sample_rate = 0;
		DWORD start_loop = 0;
		DWORD end_loop = 0;
		BYTE root_key = 60;
		CHAR pitch_correction = 0;
		SmplHandle linked_sample { 0 };
		SFSampleLink sample_type = monoSample;
	public:
		SfSampleImpl(SmplHandle handle, SampleBitDepth bit_depth)
			: self_handle{handle}, sample_bit_depth{bit_depth} {}
	};
}

SfSample::SfSample(SmplHandle handle, SampleBitDepth bit_depth) {
	pimpl = std::make_unique<SfSampleImpl>(handle, bit_depth);
}

SfSample::~SfSample() {

}

SfSample::SfSample(SfSample&& rhs) noexcept {
	this->pimpl = std::move(rhs.pimpl);
}

SfSample& SfSample::operator=(SfSample&& rhs) noexcept {
	this->pimpl = std::move(rhs.pimpl);
	return *this;
}

SfSample& SfSample::SetName(std::string_view name) {
	std::memcpy(pimpl->sample_name, &name[0], std::min<std::size_t>(name.length(), 20));
	return *this;
}

SfSample& SF2ML::SfSample::SetLoop(std::uint32_t start, std::uint32_t end) {
	pimpl->start_loop = start;
	pimpl->end_loop = end;
	return *this;
}

SfSample& SfSample::SetRootKey(std::uint8_t root_key) {
	pimpl->root_key = std::min<BYTE>(root_key, 127);
	return *this;
}

SfSample& SfSample::SetPitchCorrection(std::int8_t value) {
	pimpl->pitch_correction = value;
	return *this;
}

SfSample& SfSample::SetLink(std::optional<SmplHandle> smpl) {
	constexpr auto RomSampleFlag = RomLeftSample & RomRightSample;

	if (smpl.has_value()) {
		pimpl->linked_sample = smpl.value();
		if (pimpl->sample_type & RomSampleFlag) {
			pimpl->sample_type = RomLeftSample;
		} else {
			pimpl->sample_type = leftSample;
		}
	} else {
		pimpl->linked_sample = SmplHandle{0};
		if (pimpl->sample_type & RomSampleFlag) {
			pimpl->sample_type = RomMonoSample;
		} else {
			pimpl->sample_type = monoSample;
		}
	}
	return *this;
}

SfSample& SfSample::SetSampleMode(SFSampleLink mode) {
	pimpl->sample_type = mode;
	if (mode == monoSample || mode == RomMonoSample) {
		pimpl->linked_sample = SmplHandle{0};
	}
	return *this;
}

SfSample& SfSample::SetSampleRate(std::uint32_t smpl_rate) {
	pimpl->sample_rate = smpl_rate;
	return *this;
}

SfSample& SfSample::SetWav(std::vector<BYTE>&& wav) noexcept {
	pimpl->wav_data = std::move(wav);
	return *this;
}

SfSample& SfSample::SetWav(const std::vector<BYTE>& wav) {
	pimpl->wav_data = wav;
	return *this;
}

SmplHandle SfSample::GetHandle() const {
	return pimpl->self_handle;
}

std::string SfSample::GetName() const {
	return pimpl->sample_name;
}

const std::vector<BYTE>& SfSample::GetWav() const {
	return pimpl->wav_data;
}

int32_t SfSample::GetSampleAt(uint32_t pos) const {
	int32_t res = 0;
	if (pimpl->sample_bit_depth == SampleBitDepth::Signed16) {
		std::memcpy(&res, &pimpl->wav_data[pos * 2], 2);
	} else {
		std::memcpy(&res, &pimpl->wav_data[pos * 3], 3);
	}
	return res;
}

std::size_t SfSample::GetSampleCount() const {
	if (pimpl->sample_bit_depth == SampleBitDepth::Signed16) {
		return pimpl->wav_data.size() / 2;
	} else {
		return pimpl->wav_data.size() / 3;
	}
}

int32_t SfSample::GetSampleRate() const {
	return pimpl->sample_rate;
}

auto SfSample::GetLoop() const -> std::pair<std::uint32_t, std::uint32_t> {
	return { pimpl->start_loop, pimpl->end_loop };
}

uint8_t SfSample::GetRootKey() const {
	return pimpl->root_key;
}

int8_t SfSample::GetPitchCorrection() const {
	return pimpl->pitch_correction;
}

std::optional<SmplHandle> SfSample::GetLink() const {
	if (pimpl->sample_type & (leftSample | rightSample)) {
		return pimpl->linked_sample;
	} else {
		return std::nullopt;
	}
}

SFSampleLink SfSample::GetSampleMode() const {
	return pimpl->sample_type;
}

SampleBitDepth SfSample::GetBitDepth() const {
	return pimpl->sample_bit_depth;
}

SF2MLError SfSample::Serialize(std::ofstream& ofs) const {
	assert(false && "Not Implemented");
	return SF2MLError();
}