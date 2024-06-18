#pragma once

#include <sfml/sfspec.hpp>
#include <sfml/sfhandleinterface.hpp>
#include <sfml/sfpreset.hpp>

#include <vector>
#include <map>
#include <functional>
#include <optional>
#include <string>
#include <array>
#include <bitset>

namespace sflib {
	class InstrumentManager;

	class PresetManager {
	public:
		PresetManager(SfHandleInterface<SfPreset, PresetHandle>& presets, InstrumentManager& inst_manager)
			: presets(presets), instrument_manager(inst_manager) {} // new soundfont

		DWORD ChunkSize() const;
		SflibError Serialize(BYTE* dst, BYTE** end = nullptr) const;

		SfPreset& NewPreset(std::uint16_t preset_number, std::uint16_t bank_number, const std::string& name);
		void Remove(PresetHandle target);
	
	private:
		SfHandleInterface<SfPreset, PresetHandle>& presets;
		InstrumentManager& instrument_manager;
	};
}