#pragma once
namespace Utils {
namespace Memory {

template <size_t SIZE, class ...ARGS>
struct StaticMultiArray {

	using ThisClass = StaticMultiArray<SIZE, ARGS...>;
	using index_t = size_t;
	using Tuple = std::tuple<ARGS...>;
	using ContainerTuple = std::tuple<std::array<ARGS, SIZE>...>;

	enum {
		Size = SIZE,
		ElementSize = sizeof(Tuple),
	};

	template<size_t index>
	using ElementType = typename std::tuple_element<index, Tuple>::type;

	template <size_t index>
	typename ElementType<index>& Get(size_t itemid) {
		auto &e = std::get<index>(m_Arrays);
		return e[itemid];
	}

	size_t GetAllocatedCount() const { return m_Allocated; }

	StaticMultiArray() {}
	~StaticMultiArray() {}
	StaticMultiArray(const ThisClass&) = delete;
	StaticMultiArray(ThisClass&&) = delete;
	StaticMultiArray& operator=(const ThisClass&) = delete;
private:
	ContainerTuple m_Arrays;
};

} //namespace Memory
} //namespace Utils
