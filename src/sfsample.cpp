#include <sfml/sfsample.hpp>

using namespace sflib;

SfSample& SfSample::SetName(std::string_view name) {
	std::memset(sample_name, 0, 21);
	std::memcpy(sample_name, &name[0], std::min<std::size_t>(name.length(), 20));
	return *this;
}

SfSample& sflib::SfSample::SetLoop(std::uint32_t start, std::uint32_t end) {
	DWORD bytes_per_smpl = sample_bit_depth == SampleBitDepth::Signed16 ? 2 : 3;
	start = std::min<DWORD>(start, wav_data.size() / bytes_per_smpl);
	end = std::min<DWORD>(start, wav_data.size() / bytes_per_smpl);
	this->start_loop = start;
	this->end_loop = end;
	return *this;
}

SfSample& SfSample::SetRootKey(std::uint8_t root_key) {
	this->root_key = std::min<BYTE>(root_key, 127);
	return *this;
}

SfSample& SfSample::SetPitchCorrection(std::int8_t value) {
	this->pitch_correction = value;
	return *this;
}

SfSample& SfSample::SetLink(std::optional<SmplHandle> smpl) {
	constexpr auto RomSampleFlag = RomLeftSample & RomRightSample;

	if (smpl.has_value()) {
		this->linked_sample = smpl.value();
		if (this->sample_type & RomSampleFlag) {
			this->sample_type = RomLeftSample;
		} else {
			this->sample_type = leftSample;
		}
	} else {
		this->linked_sample = SmplHandle{0};
		if (this->sample_type & RomSampleFlag) {
			this->sample_type = RomMonoSample;
		} else {
			this->sample_type = monoSample;
		}
	}
	return *this;
}

SfSample& SfSample::SetSampleMode(SFSampleLink mode) {
	this->sample_type = mode;
	if (mode == monoSample || mode == RomMonoSample) {
		this->linked_sample = SmplHandle{0};
	}
	return *this;
}

std::string SfSample::GetName() const {
	return sample_name;
}

const std::vector<BYTE>& SfSample::GetData() const {
	return wav_data;
}

int32_t sflib::SfSample::GetSampleAt(uint32_t pos) const {
	int32_t res = 0;
	if (sample_bit_depth == SampleBitDepth::Signed16) {
		std::memcpy(&res, &wav_data[pos * 2], 2);
	} else {
		std::memcpy(&res, &wav_data[pos * 3], 3);
	}
	return res;
}

std::size_t SfSample::GetSampleCount() const {
	if (sample_bit_depth == SampleBitDepth::Signed16) {
		return wav_data.size() / 2;
	} else {
		return wav_data.size() / 3;
	}
}

int32_t sflib::SfSample::GetSampleRate() const {
	return sample_rate;
}

auto sflib::SfSample::GetLoop() const -> std::pair<std::uint32_t, std::uint32_t> {
	return { start_loop, end_loop };
}

uint8_t sflib::SfSample::GetRootKey() const {
	return root_key;
}

int8_t sflib::SfSample::GetPitchCorrection() const {
	return pitch_correction;
}

std::optional<SmplHandle> sflib::SfSample::GetLink() const {
	if (sample_type & (leftSample | rightSample)) {
		return linked_sample;
	} else {
		return std::nullopt;
	}
}

SFSampleLink sflib::SfSample::GetSampleMode() const {
	return sample_type;
}

SampleBitDepth sflib::SfSample::GetBitDepth() const {
	return sample_bit_depth;
}
