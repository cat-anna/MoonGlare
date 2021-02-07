#pragma once 

#include "../Configuration.h"

#include <unordered_map>

namespace Space {

template<class CHAR, class HASH, size_t BUFFERSTEP = 128 * 1024>
struct HashStringTable {
	static_assert(std::is_pod<CHAR>::value, "Not a pod type!");
	using Char_t = CHAR;
	using Hash_t = HASH;

	using CString = const Char_t*;
	using String = std::basic_string<Char_t>;

	using Map = std::unordered_map<Hash_t, CString>;
	using Storage = std::vector<Char_t>;

	using const_iterator = typename Map::const_iterator;

	HashStringTable() {
		m_Storage.push_back();
		m_Storage.back().reserve(BUFFERSTEP);
	}

	CString Add(Hash_t hash, CString text, size_t len = 0) {
		if (text != nullptr && len == 0) {
			len = get_len(text);
		}

		CString ptr = nullptr;
		if (text) {
			auto *storage = &m_Storage.back();
			if (storage->capacity() - storage->size() > len + 1) {
				m_Storage.push_back();
				auto *storage = &m_Storage.back();
				storage->reserve(BUFFERSTEP);
			}
			size_t pos = storage->size();
			++len;
			storage->resize(pos + len);
			ptr = (*storage)[pos];
			memcpy(ptr, text, len * sizeof(Char_t));
		}
		m_Map[hash] = ptr;
		return ptr;
	}

	CString Add(Hash_t hash, const String &text) {
		return Add(hash, text.c_str(), text.length());
	}

	CString Get(Hash_t hash) {
		auto it = m_Map.find(hash);
		if (it == m_Map.end())
			return nullptr;
		return it->second;
	}

	const_iterator begin() const { return m_Map.begin(); }
	const_iterator end() const { return m_Map.end(); }
private:
	Map m_Map;
	std::list<Storage> m_Storage;

	size_t get_len(const char *c) { return strlen(c); }
	size_t get_len(const wchar_t *c) { return wcslen(c); }
};

} //namespace Space
