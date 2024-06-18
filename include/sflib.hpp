#pragma once

#include <sfml/sfspec.hpp>
#include <sfml/sfhandle.hpp>
#include <sfml/sfsample.hpp>
#include <sfml/sfinstrument.hpp>
#include <sfml/sfpreset.hpp>
#include <sfml/sfinfo.hpp>
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

		SflibError ExportWav(std::ofstream& ofs, SmplHandle sample);

		// get sound font file meta data
		SfInfo& GetInfo();

		// set sound font file meta data
		void SetInfo(const SfInfo& info);

		// add sample to the soundfont structure
		auto AddMonoSample(
			std::ifstream& ifs,
			std::string_view name,
			SampleChannel ch=SampleChannel::Mono
		) -> SflibResult<SmplHandle>;

		auto AddMonoSample(
			const void* file_buf,
			std::size_t file_size,
			std::string_view name,
			SampleChannel ch=SampleChannel::Mono
		) -> SflibResult<SmplHandle>;

		auto AddStereoSample(
			std::ifstream& ifs,
			std::string_view left,
			std::string_view right
		) -> SflibResult<std::pair<SmplHandle, SmplHandle>>;

		auto AddStereoSample(
			const void* file_buf,
			std::size_t file_size,
			std::string_view left,
			std::string_view right
		) -> SflibResult<std::pair<SmplHandle, SmplHandle>>;

		auto LinkSamples(SmplHandle left, SmplHandle right) -> SflibError;

		auto GetSample(SmplHandle smpl) -> SfSample&;

		void RemoveSample(SmplHandle smpl, RemovalMode rm_mode = RemovalMode::Normal);

		auto FindSample(std::function<bool(const SfSample&)> pred)
		-> std::optional<SmplHandle>;

		auto FindSamples(std::function<bool(const SfSample&)> pred)
		-> std::vector<SmplHandle>;

		auto AllSamples() -> std::vector<SmplHandle>;

		SfInstrument& NewInstrument(std::string_view name);

		auto GetInstrument(InstHandle inst) -> SfInstrument&;

		void RemoveInstrument(InstHandle inst);

		auto FindInstrument(std::function<bool(const SfInstrument&)> pred)
		-> std::optional<InstHandle>;

		auto FindInstruments(std::function<bool(const SfInstrument&)> pred)
		-> std::vector<InstHandle>;
		
		// auto FindInstrumentsReferencing(SfHandle sample)
		// -> std::vector<SfHandle>;

		auto AllInstruments() -> std::vector<InstHandle>;

		SfPreset& NewPreset(
			std::uint16_t preset_number,
			std::uint16_t bank_number,
			std::string_view name
		);

		auto GetPreset(PresetHandle preset) -> SfPreset&;

		void RemovePreset(PresetHandle preset);

		auto FindPreset(std::function<bool(const SfPreset&)> pred)
		-> std::optional<PresetHandle>;

		auto FindPresets(std::function<bool(const SfPreset&)> pred)
		-> std::vector<PresetHandle>;

		// auto FindPresetsReferencing(SfHandle instruments)
		// -> std::vector<SfHandle>;

		auto AllPresets() -> std::vector<PresetHandle>;

	private:
		std::unique_ptr<class SoundFontImpl> pimpl;
	};

}