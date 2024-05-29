#pragma once

#include "sfspec.hpp"
#include "sfhandle.hpp"
#include "sfmap.hpp"
#include "sfzone.hpp"
#include <vector>
#include <map>
#include <functional>
#include <optional>
#include <string>
#include <array>
#include <bitset>

namespace sflib {
	class InstrumentManager;

	struct PresetIndex {
		WORD preset_number;
		WORD bank_number;
		auto operator<=>(const PresetIndex&) const = default;
	};

	struct PresetData {
		char preset_name[21] {};
		DWORD library;
		DWORD genre;
		DWORD morphology;
		std::optional<PresetZone> global_zone;
		std::vector<PresetZone> zones;
	};

	class PresetManager {
		std::function<void(const std::string&)> logger;
		SflibError status;
	public:
		PresetManager() {} // new soundfont
		PresetManager(const SfbkMap::Pdta& pdta);

		DWORD ChunkSize() const;
		SflibError Serialize(BYTE* dst, const InstrumentManager& inst_manager, BYTE** end = nullptr) const;

		SflibError Status() const {
			return status;
		}

		SfHandle Add(PresetIndex pid, const std::string& name);
		SflibError Remove(SfHandle target);

		// pointers are invalidated after Add/Remove operation.
		PresetData* Get(SfHandle handle) { return presets.Get(handle); }
		const PresetData* Get(SfHandle handle) const { return presets.Get(handle); }
	
		using IndexContainer1 = std::multimap<std::string, SfHandle>;
		using IndexContainer2 = std::multimap<PresetIndex, SfHandle>;

		std::optional<SfHandle> FindPreset(const std::string& name) const;
		std::optional<SfHandle> FindPreset(PresetIndex name) const;
		auto FindPresets(const std::string& name) const
			-> std::optional<std::pair<IndexContainer1::const_iterator, IndexContainer1::const_iterator>>;
		auto FindPresets(PresetIndex pid) const
			-> std::optional<std::pair<IndexContainer2::const_iterator, IndexContainer2::const_iterator>>;
	private:
		SfHandleInterface<PresetData> presets;

		// for searching
		IndexContainer1 name_index;
		IndexContainer2 pid_index;
	};
}