#pragma once

namespace Utils {
namespace Memory {

template<class T, size_t SIZE>
struct GenerationBuffer {
	using Item = T;
	enum {
		Size = SIZE,
	};

	static_assert(std::is_integral<Item>::value, "Item type for GenerationBuffer must be an integer type");

	GenerationBuffer() {
		memset(m_Memory, 0, sizeof(m_Memory));
	}

	T NewGeneration(size_t index) {
		assert(index < Size);
		return ++m_Memory[index];
	}

	T Generation(size_t index) {
		assert(index < Size);
		return m_Memory[index];
	}
private:
	T m_Memory[Size];
};

} //namespace Memory
} //namespace Utils
