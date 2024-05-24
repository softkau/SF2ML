#pragma once

#include <vector>
#include <map>
#include <algorithm>
#include <cassert>
#include "sfspec.hpp"

namespace sflib {
	struct SfHandle {
		template <typename T> friend class SfHandleInterface;
		friend class SampleManager;
		friend struct SampleData;

		auto operator<=>(const SfHandle&) const = default;
	private:
		explicit SfHandle(DWORD key=0) : key{key} {}
		DWORD key;
	};

	template <typename DataType>
	class SfHandleInterface {
	public:
		SfHandle EmplaceBack(DataType&& data) {
			SfHandle handle { next_key++ };
			interface.emplace(handle, static_cast<DWORD>(this->data.size()));
			this->data.emplace_back(std::forward<DataType>(data));
			return handle;
		}
		SfHandle PushBack(const DataType& data) {
			SfHandle handle { next_key++ };
			interface.emplace(handle, static_cast<DWORD>(this->data.size()));
			this->data.push_back(data);
			return handle;
		}
		bool Remove(SfHandle handle) {
			if (auto it = interface.find(handle); it != interface.end()) {
				DWORD id = it->second;
				assert(id < data.size() && "Dangling Handle Detected.");

				std::shift_left(data.begin() + id, data.end(), 1);
				data.pop_back();
				
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
	};

}