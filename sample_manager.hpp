#pragma once

#include "sfspec.hpp"
#include "wavspec.hpp"
#include "sfhandle.hpp"
#include <vector>
#include <map>
#include <functional>
#include <optional>
#include <string>

namespace sflib {
	using InstID = DWORD;
	using SampleID = DWORD;

	template <typename T>
	struct Ranges {
		T start;
		T end;
	};

	enum class RemovalMode {
		Restrict, Cascade, Force,
	};

	enum class SampleChannel {
		Mono, Left, Right,
	};

	struct WavInfo {
		wav::NumOfChannels num_of_channels;
		uint32_t sample_rate;
		uint16_t bit_depth;
		uint16_t block_align;
		const uint8_t* wav_data;
		uint32_t wav_size;
	};

	struct SampleData {
		char sample_name[21] {};
		std::vector<BYTE> wav_data {};
		DWORD sample_rate = 0;
		DWORD start_loop = 0;
		DWORD end_loop = 0;
		WORD root_key = 0;
		SHORT pitch_correction = 0;
		SfHandle linked_sample;
		SFSampleLink sample_type = monoSample;

		SampleData() {}
		SampleData(const SampleData&) = delete;
		SampleData& operator=(const SampleData&) = delete;
		SampleData(SampleData&&) = default;
		SampleData& operator=(SampleData&&) = default;
	};

	class SampleManager {
		std::function<void(const std::string&)> logger;
		SflibError status;
	public:
		SampleManager() {} // new soundfont
		SampleManager(const BYTE* sdta, DWORD sdta_size, const BYTE* pdta, DWORD pdta_size);

		void SetZeroZone(int count);
		DWORD ChunkSize() const;
		SflibError SerializeSDTA(BYTE* dst, BYTE** end = nullptr) const;
		SflibError SerializeSHDR(BYTE* dst, BYTE** end = nullptr) const;

		SflibError Status() const {
			return status;
		}

		SflibResult<SfHandle> AddMono(const void* wav_data, std::size_t wav_size,
			std::optional<std::string> name = std::nullopt,
			std::optional<Ranges<uint32_t>> loop = std::nullopt,
			std::optional<uint16_t> root_key = std::nullopt,
			std::optional<int16_t> pitch_correction = std::nullopt,
			SampleChannel sample_type = SampleChannel::Mono
		);

		SflibResult<SfHandle> AddStereo(const void* wav_data, size_t wav_size,
			std::optional<std::string> name = std::nullopt,
			std::optional<Ranges<uint32_t>> loop = std::nullopt,
			std::optional<uint16_t> root_key = std::nullopt,
			std::optional<int16_t> pitch_correction = std::nullopt
		);

		SflibError Remove(SfHandle target, RemovalMode mode=RemovalMode::Restrict);

		SflibError LinkStereo(SfHandle left, SfHandle right);

		void SetName(SfHandle target, const std::string& value);
		void SetLoopPoint(SfHandle target, uint32_t start, uint32_t end);
		void SetRootKey(SfHandle target, uint16_t value);
		void SetPitchCorrection(SfHandle target, int16_t value);

		std::string GetName(SfHandle target) const;
		Ranges<uint32_t> GetLoopPoint(SfHandle target) const;
		uint16_t GetRootKey(SfHandle target) const;
		int16_t GetPitchCorrection(SfHandle target) const;
		int32_t GetSampleRate(SfHandle target) const;
		SfHandle GetLinkedSample(SfHandle target) const;
		bool IsMono(SfHandle target) const;
		bool IsLeftChannel(SfHandle target) const;
		bool IsRightChannel(SfHandle target) const;
		bool IsRomSample(SfHandle target) const;
		static SflibResult<WavInfo> ValidateWav(const void* data, size_t size);

		SflibError AddRef(SfHandle smpl, InstID inst);
		SflibError RemoveRef(SfHandle smpl, InstID inst);

		std::optional<SfHandle> FindSample(const std::string& name) const {
			if (auto it = smpl_index.find(name); it != smpl_index.end()) {
				return it->second;
			}
			return {};
		}

		using IndexContainer = std::multimap<std::string, SfHandle>;
		auto FindSamples(const std::string& name) const -> std::optional<std::pair<IndexContainer::const_iterator, IndexContainer::const_iterator>> {
			auto res = smpl_index.equal_range(name);
			if (res.first != res.second) {
				return res;
			} else {
				return {};
			}
		}

	private:
		int sample_bitdepth = 16;
		int z_zone = 46; // zero zone size (which exists in between sample data)
		SfHandleInterface<SampleData> samples;
		std::multimap<SfHandle, InstID> referenced; // keeps track of instruments referencing the sample(s)

		IndexContainer smpl_index; // for searching samples by name
	};

}