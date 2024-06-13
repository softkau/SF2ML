#pragma once

#include "sfspec.hpp"
#include "sfhandle.hpp"
#include "sfsample.hpp"
#include "sfinstrument.hpp"
#include "sfpreset.hpp"
#include "sfinfo.hpp"
#include <memory>
#include <functional>
#include <fstream>
#include <cstddef>

namespace sflib {

	class SoundFont {
	public:
		// create new soundfont structure
		SoundFont();
		~SoundFont();

		// reset soundfont structure by loading existing sf2 file
		SflibError Load(std::ifstream& ifs);

		// save soundfont structure to disk in .sf2 binary format
		SflibError Save(std::ofstream& ofs);

		SflibError ExportWav(std::ofstream& ofs, SfHandle sample);

		// get sound font file meta data
		SfInfo& GetInfo();

		// set sound font file meta data
		void SetInfo(const SfInfo& info);

		// add sample to the soundfont structure
		auto AddMonoSample(
			std::ifstream& ifs,
			std::string_view name,
			SampleChannel ch=SampleChannel::Mono
		) -> SflibResult<SfHandle>;

		auto AddMonoSample(
			const void* file_buf,
			std::size_t file_size,
			std::string_view name,
			SampleChannel ch=SampleChannel::Mono
		) -> SflibResult<SfHandle>;

		auto AddStereoSample(
			std::ifstream& ifs,
			std::string_view left,
			std::string_view right
		) -> SflibResult<std::pair<SfHandle, SfHandle>>;

		auto AddStereoSample(
			const void* file_buf,
			std::size_t file_size,
			std::string_view left,
			std::string_view right
		) -> SflibResult<std::pair<SfHandle, SfHandle>>;

		auto LinkSamples(SfHandle left, SfHandle right) -> SflibError;

		auto GetSample(SfHandle smpl) -> SfSample&;

		void RemoveSample(SfHandle smpl, RemovalMode rm_mode = RemovalMode::Normal);

		auto FindSample(std::function<bool(const SfSample&)> pred)
		-> std::optional<SfHandle>;

		auto FindSamples(std::function<bool(const SfSample&)> pred)
		-> std::vector<SfHandle>;

		auto AllSamples() -> std::vector<SfHandle>;

		SfInstrument& NewInstrument(std::string_view name);

		auto GetInstrument(SfHandle inst) -> SfInstrument&;

		void RemoveInstrument(SfHandle inst, RemovalMode rm_mode=RemovalMode::Normal);

		auto FindInstrument(std::function<bool(const SfInstrument&)> pred)
		-> std::optional<SfHandle>;

		auto FindInstruments(std::function<bool(const SfInstrument&)> pred)
		-> std::vector<SfHandle>;
		
		// auto FindInstrumentsReferencing(SfHandle sample)
		// -> std::vector<SfHandle>;

		auto AllInstruments() -> std::vector<SfHandle>;

		SfPreset& NewPreset(
			std::uint16_t preset_number,
			std::uint16_t bank_number,
			std::string_view name
		);

		auto GetPreset(SfHandle preset) -> SfPreset&;

		void RemovePreset(SfHandle preset, RemovalMode rm_mode=RemovalMode::Normal);

		auto FindPreset(std::function<bool(const SfPreset&)> pred)
		-> std::optional<SfHandle>;

		auto FindPresets(std::function<bool(const SfPreset&)> pred)
		-> std::vector<SfHandle>;

		// auto FindPresetsReferencing(SfHandle instruments)
		// -> std::vector<SfHandle>;

		auto AllPresets() -> std::vector<SfHandle>;

	private:
		std::unique_ptr<class SoundFontImpl> pimpl;
	};

}