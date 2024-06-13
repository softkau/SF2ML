#pragma once

#include "sfspec.hpp"
#include "sfhandle.hpp"
#include "sfmap.hpp"
#include "sfinstrument.hpp"
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

	class InstrumentManager {
	public:
		InstrumentManager(SfHandleInterface<SfInstrument>& insts,
		                  SampleManager& sample_manager,
		                  std::map<SfHandle, int>& sample_reference_count)
			: insts(insts), sample_manager(sample_manager), smpl_ref_count(sample_reference_count) {}

		DWORD ChunkSize() const;
		SflibError Serialize(BYTE* dst, BYTE** end = nullptr) const;

		SfInstrument& NewInstrument(const std::string& name);
		void Remove(SfHandle target, RemovalMode rm_mode=RemovalMode::Normal);
		
		// should only be used for serializing purposes
		std::optional<InstID> GetInstID(SfHandle target) const;
	private:
		SfHandleInterface<SfInstrument>& insts;
		SampleManager& sample_manager;
		std::map<SfHandle, int>& smpl_ref_count;
	};
}