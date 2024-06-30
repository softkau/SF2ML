#ifndef SF2ML_SFSAMPLE_HPP_
#define SF2ML_SFSAMPLE_HPP_

#include "wavspec.hpp"
#include "sfspec.hpp"
#include "sfhandle.hpp"

#include <cstdint>
#include <memory>
#include <optional>
#include <fstream>
#include <vector>
#include <string>
#include <string_view>

namespace SF2ML {
	class SfSample {
	public:
		SfSample(SmplHandle handle, SampleBitDepth bit_depth);
		~SfSample();
		SfSample(const SfSample&) = delete;
		SfSample(SfSample&& rhs) noexcept;
		SfSample& operator=(const SfSample&) = delete;
		SfSample& operator=(SfSample&&) noexcept;

		SfSample& SetName(std::string_view name);
		SfSample& SetLoop(std::uint32_t start, std::uint32_t end);
		SfSample& SetRootKey(std::uint8_t root_key);
		SfSample& SetPitchCorrection(std::int8_t value);
		SfSample& SetLink(std::optional<SmplHandle> smpl);
		SfSample& SetSampleMode(SFSampleLink mode);
		SfSample& SetSampleRate(std::uint32_t smpl_rate);
		SfSample& SetWav(std::vector<BYTE>&& wav) noexcept;
		SfSample& SetWav(const std::vector<BYTE>& wav);

		SmplHandle GetHandle() const;

		std::string GetName() const;
		const std::vector<BYTE>& GetWav() const;
		int32_t GetSampleAt(uint32_t pos) const;
		std::size_t GetSampleCount() const;
		int32_t GetSampleRate() const;
		auto GetLoop() const -> std::pair<std::uint32_t, std::uint32_t>;
		uint8_t GetRootKey() const;
		int8_t GetPitchCorrection() const;
		std::optional<SmplHandle> GetLink() const;
		SFSampleLink GetSampleMode() const;
		SampleBitDepth GetBitDepth() const;

		SF2MLError Serialize(std::ofstream& ofs) const;
	private:
		std::unique_ptr<class SfSampleImpl> pimpl;
	};
}

#endif