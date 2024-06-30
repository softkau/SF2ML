#ifndef SF2ML_SF2ML_HPP_
#define SF2ML_SF2ML_HPP_

#include "sftypes.hpp"
#include "sfspec.hpp"
#include "sfhandle.hpp"
#include "sfsample.hpp"
#include "sfinstrument.hpp"
#include "sfpreset.hpp"
#include "sfinfo.hpp"

#include <cstddef>
#include <memory>
#include <functional>
#include <fstream>
#include <vector>
#include <string>
#include <string_view>
#include <optional>

namespace SF2ML {
	enum class RemovalMode {
		Recursive, Normal
	};

	enum class SampleChannel {
		Mono, Left, Right,
	};

	class SoundFont {
	public:
		/// @brief Creates a new SoundFont object.
		SoundFont();


		/// @brief Deletes the SoundFont object.
		///        Handles created from this object will also be invalidated.
		~SoundFont();


		/// @brief Loads .sf2 file from file stream.
		///        The SoundFont object will be reinitialized according to the file.
		///        Handles created from this object will also be invalidated when the method is called.
		///        (doc not finished!)
		/// @param ifs The file stream for .sf2 file to load.
		///            The behavior is undefined if (ifs.is_open() == false).
		/// @retval SF2ML::SF2ML_SUCCESS when success
		/// @retval SF2ML::SF2ML_FAILED when failed
		auto Load(std::ifstream& ifs) -> SF2MLError;


		/// @brief Saves the SoundFont object to disk
		/// @param ofs The file stream for .sf2 file to save.
		///            The behavior is undefined if (ofs.is_open() == false).
		/// @retval SF2ML::SF2ML_SUCCESS when success
		/// @retval SF2ML::SF2ML_FAILED when failed
		auto Save(std::ofstream& ofs) -> SF2MLError;


		/// @brief Exports the SfSample object existing in SoundFont object as .WAV file to disk.
		/// @note currently unimplemented.
		/// @param ofs The file stream for .WAV file.
		///            The behavior is undefined if (ofs.is_open() == false).
		/// @param sample Handle of the SfSample to export.
		/// @retval SF2ML::SF2ML_SUCCESS when success
		auto ExportWav(std::ofstream& ofs, SmplHandle sample) -> SF2MLError;

		/// @brief Gets the meta-data object of Soundfont object.
		///        It corrisponds to INFO chunk described in [SoundFont Technical Specification]*
		/// @return Reference to SfInfo object
		SfInfo& Info();


		/// @brief Adds a mono-channel sample to the SoundFont object.
		///        When there's non-zero amount of samples in the SoundFont object,
		///        the .WAV sample to be loaded in has to be in the same bit-depth as
		///        of existing samples in the SoundFont object.
		///        Otherwise, it will fail to load the sample.
		///        If the .WAV sample is in some unsupported format(not plain-old PCM),
		///        it will also failed to load.
		///        The object is still a valid, properly defined state even after the failure.
		/// @param ifs The file stream for the .WAV sample.
		/// @param name sample name
		/// @param loop loop point, in sample units (if std::nullopt, it'll be set to [sample start, sample end])
		/// @param root_key root_key, in MIDI key units (if std::nullopt, it'll be set to 60(=C4))
		/// @param pitch_correction pitch_correction value in cents (default = 0)
		/// @param ch the sample channel to load.
		///           Use SampleChannel::Mono, if the .WAV file is in mono.
		///           Use SampleChannel::Left or SampleChannel::Right if the .WAV file is in stereo.
		///           If SampleChannel::Left, only the left channel is loaded.
		///           If SampleChannel::Right, only the right channel is loaded.
		///           Note that regardless of ch, the resulting SfSample object has the SampleMode of 'monoSample'.
		/// @retval When succeeded, SF2MLResult::value will contain SmplHandle of the newly added SfSample object,
		///         and SF2MLResult::error will be SF2ML::SF2ML_SUCCESS
		/// @retval When failed, SF2MLResult::error will contain error codes
		auto AddMonoSample(
			std::ifstream& ifs,
			std::string_view name,
			std::optional<Ranges<DWORD>> loop = std::nullopt,
			std::optional<BYTE> root_key = std::nullopt,
			std::optional<CHAR> pitch_correction = std::nullopt,
			SampleChannel ch=SampleChannel::Mono
		) -> SF2MLResult<SmplHandle>;


		/// @brief This function is provided for convenience, and it basically does the same thing as the function above.
		auto AddMonoSample(
			const void* file_buf,
			std::size_t file_size,
			std::string_view name,
			std::optional<Ranges<DWORD>> loop = std::nullopt,
			std::optional<BYTE> root_key = std::nullopt,
			std::optional<CHAR> pitch_correction = std::nullopt,
			SampleChannel ch=SampleChannel::Mono
		) -> SF2MLResult<SmplHandle>;


		/// @brief This function is provided for conveniently creating
		///        2 SfSample objects, linked together, from Stereo WAV file.
		/// @param ifs The file stream for the .WAV sample.
		/// @param left sample name for left channel
		/// @param right sample name for right channel
		/// @param loop loop point, in sample units (if std::nullopt, it'll be set to [sample start, sample end])
		/// @param root_key root_key, in MIDI key units (if std::nullopt, it'll be set to 60(=C4))
		/// @param pitch_correction pitch_correction value in cents (default = 0)
		/// @retval When succeeded, SF2MLResult::value will contain pairs of
		///         newly created SfSample object(first=left, second=right),
		///         and SF2MLResult::error will be SF2ML::SF2ML_SUCCESS
		/// @retval When failed, SF2MLResult::error will contain error codes
		auto AddStereoSample(
			std::ifstream& ifs,
			std::string_view left,
			std::string_view right,
			std::optional<Ranges<DWORD>> loop = std::nullopt,
			std::optional<BYTE> root_key = std::nullopt,
			std::optional<CHAR> pitch_correction = std::nullopt
		) -> SF2MLResult<std::pair<SmplHandle, SmplHandle>>;


		/// @brief This function is provided for convenience, and it basically does the same thing as the function above.
		auto AddStereoSample(
			const void* file_buf,
			std::size_t file_size,
			std::string_view left,
			std::string_view right,
			std::optional<Ranges<DWORD>> loop = std::nullopt,
			std::optional<BYTE> root_key = std::nullopt,
			std::optional<CHAR> pitch_correction = std::nullopt
		) -> SF2MLResult<std::pair<SmplHandle, SmplHandle>>;


		/// @brief Links two samples. If the properties of the samples do not match,
		///        it'll fail to link. No modification done when failed.
		/// @param left SmplHandle of left sample
		/// @param right SmplHandle of right sample
		/// @retval SF2ML::SF2ML_BAD_LINK when the properties don't match(ex: loop points, sample rates, etc)
		/// @retval SF2ML::SF2ML_SUCCESS when succeeded
		auto LinkSamples(SmplHandle left, SmplHandle right) -> SF2MLError;

		
		/// @brief Gets the reference of corresponding SfSample object.
		///        The behavior is undefined if the handle is invalid.
		///        Note that after addition/deletion of the SfSample,
		///        the reference obtained before the addition/deletion is invalidated,
		///        and you need to get the valid reference again via calling this method.
		/// @param smpl the handle of the SfSample object
		/// @return reference to the SfSample object
		auto GetSample(SmplHandle smpl) -> SfSample&;


		/// @brief Removes the SfSample object with corrisponding handle
		///        If the SfSample does not exist, it'll do nothing.
		/// @param smpl the handle of the SfSample object
		/// @param rm_mode if RemovalMode::Recursive, it'll also remove the linked SfSample object.
		void RemoveSample(SmplHandle smpl, RemovalMode rm_mode = RemovalMode::Normal);


		/// @brief Find 1 SfSample object that satisfies pred(object) == true.
		/// @param pred Functor that evaluates the object to bool
		/// @return std::nullopt when none satisfies the condition.
		auto FindSample(std::function<bool(const SfSample&)> pred)
		-> std::optional<SmplHandle>;


		/// @brief Find all SfSample objects that satisfies pred(object) == true.
		/// @param pred Functor that evaluates the object to bool
		/// @return a vector of SmplHandles
		auto FindSamples(std::function<bool(const SfSample&)> pred)
		-> std::vector<SmplHandle>;


		/// @brief Returns all SfSample objects that exists in the SoundFont object.
		/// @return a vector of SmplHandles
		auto AllSamples() -> std::vector<SmplHandle>;


		/// @brief Creates new SfInstrument object in the SoundFont object.
		///        The returned reference is "short lived", which means
		///        whenever addition/deletion of SfInstrument objects in SoundFont object,
		///        the reference obtained before the addition/deletion is invalidated.
		/// @param name name of the SfInstrument
		/// @return reference to newly created SfInstrument
		SfInstrument& NewInstrument(std::string_view name);


		/// @brief Gets the reference of corresponding SfInstrument object.
		///        The behavior is undefined if the handle is invalid.
		///        Note that after addition/deletion of the SfInstrument,
		///        the reference obtained before the addition/deletion is invalidated,
		///        and you need to get the valid reference again via calling this method.
		/// @param inst the handle of the SfInstrument object
		/// @return reference to the SfInstrument object
		auto GetInstrument(InstHandle inst) -> SfInstrument&;


		/// @brief Removes the SfInstrument object with corrisponding handle
		///        If the SfInstrument does not exist, it'll do nothing.
		/// @param inst the handle of the SfInstrument object
		void RemoveInstrument(InstHandle inst);


		/// @brief Find 1 SfInstrument object that satisfies pred(object) == true.
		/// @param pred Functor that evaluates the object to bool
		/// @return std::nullopt when none satisfies the condition.
		auto FindInstrument(std::function<bool(const SfInstrument&)> pred)
		-> std::optional<InstHandle>;


		/// @brief Find all SfInstrument objects that satisfies pred(object) == true.
		/// @param pred Functor that evaluates the object to bool
		/// @return a vector of InstHandles
		auto FindInstruments(std::function<bool(const SfInstrument&)> pred)
		-> std::vector<InstHandle>;


		/// @brief Returns all SfInstrument objects that exists in the SoundFont object.
		/// @return a vector of InstHandles
		auto AllInstruments() -> std::vector<InstHandle>;


		/// @brief Creates new SfPreset object in the SoundFont object.
		///        The returned reference is "short lived", which means
		///        whenever addition/deletion of SfPreset objects in SoundFont object,
		///        the reference obtained before the addition/deletion is invalidated.
		/// @param preset_number preset number
		/// @param bank_number bank number
		/// @param name name of the SfPreset
		/// @return reference to newly created SfPreset
		SfPreset& NewPreset(
			std::uint16_t preset_number,
			std::uint16_t bank_number,
			std::string_view name
		);


		/// @brief Gets the reference of corresponding SfPreset object.
		///        The behavior is undefined if the handle is invalid.
		///        Note that after addition/deletion of the SfPreset,
		///        the reference obtained before the addition/deletion is invalidated,
		///        and you need to get the valid reference again via calling this method.
		/// @param preset the handle of the SfPreset object
		/// @return reference to the SfPreset object
		auto GetPreset(PresetHandle preset) -> SfPreset&;


		/// @brief Removes the SfPreset object with corrisponding handle
		///        If the SfPreset does not exist, it'll do nothing.
		/// @param preset the handle of the SfPreset object
		void RemovePreset(PresetHandle preset);


		/// @brief Find 1 SfPreset object that satisfies pred(object) == true.
		/// @param pred Functor that evaluates the object to bool
		/// @return std::nullopt when none satisfies the condition.
		auto FindPreset(std::function<bool(const SfPreset&)> pred)
		-> std::optional<PresetHandle>;


		/// @brief Find all SfPreset objects that satisfies pred(object) == true.
		/// @param pred Functor that evaluates the object to bool
		/// @return a vector of PresetHandles
		auto FindPresets(std::function<bool(const SfPreset&)> pred)
		-> std::vector<PresetHandle>;


		/// @brief Returns all SfPreset objects that exists in the SoundFont object.
		/// @return a vector of PresetHandles
		auto AllPresets() -> std::vector<PresetHandle>;

	private:
		std::unique_ptr<class SoundFontImpl> pimpl;
	};

}

#endif