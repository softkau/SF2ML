#ifndef SF2ML_SFHANDLEINTERFACE_HPP_
#define SF2ML_SFHANDLEINTERFACE_HPP_

#include <vector>
#include <map>
#include <algorithm>
#include <optional>
#include <cassert>
#include <utility>
#include <limits>
#include <concepts>
#include "sfspec.hpp"
#include "sfhandle.hpp"

namespace SF2ML {

	template <typename T>
	concept SfHandle = std::totally_ordered<T>
					&& (std::same_as<T, SmplHandle>
	                || std::same_as<T, InstHandle>
					|| std::same_as<T, PresetHandle>
					|| std::same_as<T, IZoneHandle>
					|| std::same_as<T, PZoneHandle>);

	template <typename T, typename HandleType>
	concept DataTypeRequirements = requires (T x, HandleType handle) {
		{x.GetHandle()} -> std::same_as<HandleType>;
	};

	template <typename DataType, typename HandleT>
	requires SfHandle<HandleT> && DataTypeRequirements<DataType, HandleT>
	class SfHandleInterface {
	public:
		/** @brief creates new item with corresponding new handle
		 *  @throw throws std::length_error when item can no longer be created
		 *  @return reference to the newly created object
		*/
		template <typename... Args>
		DataType& NewItem(Args&&... args) {
			if (data.size() >= std::numeric_limits<decltype(HandleT::value)>::max()) {
				throw std::length_error("Cannot create more items!");
			}

			while (interface.find(HandleT(next_key)) != interface.end()) {
				++next_key;
			}

			HandleT handle(next_key++);
			interface.emplace(handle, static_cast<DWORD>(this->data.size()));
			data.emplace_back(handle, std::forward<Args>(args)...);
			handles.push_back(handle);
			return data.back();
		}

		/** @brief removes item from interface with corresponding handle
		 *  @return true when succeeded, false when failed(due to removal of non existing item)
		*/
		bool Remove(HandleT handle) noexcept {
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

		/** @brief gets pointer to the item with corresponding handle (invalidated when NewItem/Remove is called)
		 *  @return pointer to the existing item / nullptr when it does not exist
		 */
		DataType* Get(HandleT handle) noexcept {
			if (auto it = interface.find(handle); it != interface.end()) {
				DWORD id = it->second;
				assert(id < data.size() && "Dangling Handle Detected.");
				return &data[id];
			}
			return nullptr;
		}

		/** @brief gets constant pointer to the item with corresponding handle (invalidated when NewItem/Remove is called)
		 *  @return constant pointer to the existing item / nullptr when it does not exist
		 */
		const DataType* Get(HandleT handle) const noexcept {
			if (auto it = interface.find(handle); it != interface.end()) {
				DWORD id = it->second;
				assert(id < data.size() && "Dangling Handle Detected.");
				return &data[id];
			}
			return nullptr;
		}

		// should only be used for serializing purposes.
		std::optional<DWORD> GetID(HandleT handle) const noexcept {
			if (auto it = interface.find(handle); it != interface.end()) {
				DWORD id = it->second;
				return id;
			}
			return std::nullopt;
		}

		/** @brief gets all valid handles from interface (there is no guarantee in ordering)
		 *  @return first, last const_iterator pair of all valid handles
		*/
		auto GetAllHandles() const noexcept
		-> std::pair<typename std::vector<HandleT>::const_iterator, typename std::vector<HandleT>::const_iterator> {
			return std::make_pair(handles.cbegin(), handles.cend());
		}

		// @brief counts all items in interface
		DWORD Count() const noexcept {
			return data.size();
		}

		// @breif counts all items that satisfies "pred(item) == true"
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

		auto begin() noexcept -> typename std::vector<DataType>::iterator {
			return data.begin();
		}
		auto end() noexcept -> typename std::vector<DataType>::iterator {
			return data.end();
		}
		auto begin() const noexcept -> typename std::vector<DataType>::const_iterator {
			return data.begin();
		}
		auto end() const noexcept -> typename std::vector<DataType>::const_iterator {
			return data.end();
		}
	private:
		decltype(HandleT::value) next_key = 0;
		std::map<HandleT, DWORD> interface;
		std::vector<DataType> data;
		std::vector<HandleT> handles;
	};
}

#endif