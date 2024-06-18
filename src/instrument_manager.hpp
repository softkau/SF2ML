#pragma once

#include <sfml/sfspec.hpp>
#include <sfml/sfhandleinterface.hpp>
#include <sfml/sfinstrument.hpp>

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
		InstrumentManager(SfHandleInterface<SfInstrument, InstHandle>& insts,
		                  SampleManager& sample_manager)
			: insts(insts), sample_manager(sample_manager) {}

		DWORD ChunkSize() const;
		SflibError Serialize(BYTE* dst, BYTE** end = nullptr) const;

		SfInstrument& NewInstrument(const std::string& name);
		void Remove(InstHandle target);
		
		// should only be used for serializing purposes
		std::optional<InstID> GetInstID(InstHandle target) const;
	private:
		SfHandleInterface<SfInstrument, InstHandle>& insts;
		SampleManager& sample_manager;
	};
}