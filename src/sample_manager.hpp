#pragma once

#include <sfml/sfspec.hpp>
#include <sfml/wavspec.hpp>
#include <sfml/sfhandleinterface.hpp>
#include <sfml/sfsample.hpp>

#include <optional>
#include <string>

namespace sflib {
	using SampleID = DWORD;

	class SampleManager {
	public:
		SampleManager(SfHandleInterface<SfSample, SmplHandle>& samples, SampleBitDepth& bit_depth)
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
		) -> SflibResult<SmplHandle>;

		auto AddStereo(
			const void* wav_data,
			size_t wav_size,
			const std::string& name_left,
			const std::string& name_right,
			std::optional<Ranges<uint32_t>> loop = std::nullopt,
			std::optional<uint16_t> root_key = std::nullopt,
			std::optional<int16_t> pitch_correction = std::nullopt
		) -> SflibResult<std::pair<SmplHandle, SmplHandle>>;

		SfSample* Get(SmplHandle handle) { return samples.Get(handle); }

		void Remove(SmplHandle target, RemovalMode rm_mode);

		SflibError LinkStereo(SmplHandle left, SmplHandle right);

		static SflibResult<WavInfo> ValidateWav(const void* data, size_t size);

		// should only be used for serializing purposes
		std::optional<SampleID> GetSampleID(SmplHandle target) const;

		// TODO: mehtod to change sample bit depth

	private:
		SfHandleInterface<SfSample, SmplHandle>& samples;
		SampleBitDepth& bit_depth;
		int z_zone = 46; // zero zone size (which exists in between sample data)
	};

}