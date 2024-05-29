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
	using InstID = DWORD;
	class SampleManager;

	struct InstData {
		char inst_name[21] {};
		std::optional<InstZone> global_zone;
		std::vector<InstZone> zones;
	};

	class InstrumentManager {
		std::function<void(const std::string&)> logger;
		SflibError status;
	public:
		InstrumentManager() {} // new soundfont
		InstrumentManager(const SfbkMap::Pdta& pdta);

		DWORD ChunkSize() const;
		SflibError Serialize(BYTE* dst, const SampleManager& sample_manager, BYTE** end = nullptr) const;

		SflibError Status() const {
			return status;
		}

		SfHandle Add(const std::string& name);
		SflibError Remove(SfHandle target);

		// pointers are invalidated after Add/Remove operation.
		InstData* Get(SfHandle handle) { return insts.Get(handle); }
		const InstData* Get(SfHandle handle) const { return insts.Get(handle); }

		// should only be used for serializing purposes
		std::optional<InstID> GetInstID(SfHandle target) const;

		std::optional<SfHandle> FindInst(const std::string& name) const;
		
		using IndexContainer = std::multimap<std::string, SfHandle>;
		auto FindInsts(const std::string& name) const
			-> std::optional<std::pair<IndexContainer::const_iterator, IndexContainer::const_iterator>>;
	private:
		SfHandleInterface<InstData> insts;

		IndexContainer inst_index;
	};
}