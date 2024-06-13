#pragma once

#include "sfspec.hpp"
#include "wavspec.hpp"
#include "sfhandle.hpp"
#include "sfsample.hpp"
#include <optional>
#include <string>

namespace sflib {
	using SampleID = DWORD;

	class SampleManager {
	public:
		SampleManager(SfHandleInterface<SfSample>& samples, SampleBitDepth bit_depth)
			: samples(samples), bit_depth(bit_depth) {}

		void SetZeroZone(int count);
		DWORD SdtaSize() const;
		DWORD ShdrSize() const;
		SflibError SerializeSDTA(BYTE* dst, BYTE** end = nullptr) const;
		SflibError SerializeSHDR(BYTE* dst, BYTE** end = nullptr) const;

		auto AddMono(
			const void* wav_data,
			std::size_t wav_size,
			const std::string& name,
			std::optional<Ranges<uint32_t>> loop = std::nullopt,
			std::optional<uint16_t> root_key = std::nullopt,
			std::optional<int16_t> pitch_correction = std::nullopt,
			SampleChannel sample_type = SampleChannel::Mono
		) -> SflibResult<SfHandle>;

		auto AddStereo(
			const void* wav_data,
			size_t wav_size,
			const std::string& name_left,
			const std::string& name_right,
			std::optional<Ranges<uint32_t>> loop = std::nullopt,
			std::optional<uint16_t> root_key = std::nullopt,
			std::optional<int16_t> pitch_correction = std::nullopt
		) -> SflibResult<std::pair<SfHandle, SfHandle>>;

		SfSample* Get(SfHandle handle) { return samples.Get(handle); }

		void Remove(SfHandle target, RemovalMode rm_mode);

		SflibError LinkStereo(SfHandle left, SfHandle right);

		static SflibResult<WavInfo> ValidateWav(const void* data, size_t size);

		// should only be used for serializing purposes
		std::optional<SampleID> GetSampleID(SfHandle target) const;

	private:
		SfHandleInterface<SfSample>& samples;
		SampleBitDepth bit_depth = SampleBitDepth::Signed16;
		int z_zone = 46; // zero zone size (which exists in between sample data)
	};

}