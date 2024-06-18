#pragma once

#include "wavspec.hpp"
#include "sfspec.hpp"
#include "sfhandle.hpp"
#include <vector>
#include <string_view>
#include <fstream>
#include <cstdint>
#include <optional>

namespace sflib {
	enum class SampleBitDepth {
		Signed16, Signed24
	};

	struct WavInfo {
		wav::NumOfChannels num_of_channels;
		uint32_t sample_rate;
		SampleBitDepth bit_depth;
		uint16_t block_align;
		const uint8_t* wav_data;
		uint32_t wav_size;
	};

	class SfSample {
	public:
		SfSample(SmplHandle handle) : self_handle(handle) {}

		SfSample& SetName(std::string_view name);
		SfSample& SetLoop(std::uint32_t start, std::uint32_t end);
		SfSample& SetRootKey(std::uint8_t root_key);
		SfSample& SetPitchCorrection(std::int8_t value);
		SfSample& SetLink(std::optional<SmplHandle> smpl);
		SfSample& SetSampleMode(SFSampleLink mode);

		SmplHandle GetHandle() const { return self_handle; }

		std::string GetName() const;
		const std::vector<BYTE>& GetData() const;
		int32_t GetSampleAt(uint32_t pos) const;
		std::size_t GetSampleCount() const;
		int32_t GetSampleRate() const;
		auto GetLoop() const -> std::pair<std::uint32_t, std::uint32_t>;
		uint8_t GetRootKey() const;
		int8_t GetPitchCorrection() const;
		std::optional<SmplHandle> GetLink() const;
		SFSampleLink GetSampleMode() const;
		SampleBitDepth GetBitDepth() const;
	private:
		SmplHandle self_handle;

		char sample_name[21] {};
		std::vector<BYTE> wav_data {};
		DWORD sample_rate = 0;
		DWORD start_loop = 0;
		DWORD end_loop = 0;
		WORD root_key = 60;
		SHORT pitch_correction = 0;
		SmplHandle linked_sample { 0 };
		SFSampleLink sample_type = monoSample;
		SampleBitDepth sample_bit_depth = SampleBitDepth::Signed16;
	
		friend class SoundFontImpl;
		friend class SampleManager;
	};
}