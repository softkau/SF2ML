#pragma once

#include <vector>
#include <map>
#include <algorithm>
#include <optional>
#include <cassert>
#include "sfspec.hpp"

namespace sflib {
	using SfHandle = uint32_t;

	template <typename DataType>
	class SfHandleInterface {
	public:
		DataType& NewItem() {
			SfHandle handle { next_key++ };
			interface.emplace(handle, static_cast<DWORD>(this->data.size()));
			handles.push_back(handle);
			data.emplace_back(handle);
			return data.back();
		}
		
		bool Remove(SfHandle handle) {
			if (auto it = interface.find(handle); it != interface.end()) {
				DWORD id = it->second;
				assert(id < data.size() && "Dangling Handle Detected.");

				std::shift_left(data.begin() + id, data.end(), 1);
				data.pop_back();

				auto it_handle = std::find(handles.begin(), handles.end(), handle);
				handles.erase(it_handle);
				
				interface.erase(it);
				for (auto& m : interface) {
					if (m.second > id) {
						m.second--;
					}
				}
				return true;
			}
			return false;
		}

		DataType* Get(SfHandle handle) {
			if (auto it = interface.find(handle); it != interface.end()) {
				DWORD id = it->second;
				assert(id < data.size() && "Dangling Handle Detected.");
				return &data[id];
			}
			return nullptr;
		}

		const DataType* Get(SfHandle handle) const {
			if (auto it = interface.find(handle); it != interface.end()) {
				DWORD id = it->second;
				assert(id < data.size() && "Dangling Handle Detected.");
				return &data[id];
			}
			return nullptr;
		}

		// should only be used for serializing purposes.
		std::optional<DWORD> GetID(SfHandle handle) const {
			if (auto it = interface.find(handle); it != interface.end()) {
				DWORD id = it->second;
				return id;
			}
			return std::nullopt;
		}

		auto GetAllHandles() const
		-> std::pair<std::vector<SfHandle>::const_iterator, std::vector<SfHandle>::const_iterator> {
			return std::make_pair(handles.cbegin(), handles.cend());
		}

		DWORD Count() const {
			return data.size();
		}

		template <typename Functor>
		DWORD CountIf(Functor pred) const {
			DWORD sz = 0;
			for (const auto& x : data) {
				if (pred(x)) {
					sz++;
				}
			}
			return sz;
		}

		auto begin() -> typename std::vector<DataType>::iterator {
			return data.begin();
		}
		auto end() -> typename std::vector<DataType>::iterator {
			return data.end();
		}
		auto begin() const -> typename std::vector<DataType>::const_iterator {
			return data.begin();
		}
		auto end() const -> typename std::vector<DataType>::const_iterator {
			return data.end();
		}
	private:
		DWORD next_key = 0;
		std::map<SfHandle, DWORD> interface;
		std::vector<DataType> data;
		std::vector<SfHandle> handles;
	};

}