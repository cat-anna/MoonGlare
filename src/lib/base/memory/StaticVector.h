#pragma once

namespace MoonGlare::Memory {

template<class ELEMENT, size_t SIZE>
struct StaticVector final {
    using ArrayType = std::array<ELEMENT, SIZE>;
    using reference = typename ArrayType::reference;
    using const_reference = typename ArrayType::const_reference;
    using iterator = typename ArrayType::iterator;
    using const_iterator = typename ArrayType::const_iterator;
    using value_type = ELEMENT;
    using size_type = typename ArrayType::size_type;

    StaticVector(): m_Allocated(0) { }

    size_t Allocated() const { return m_Allocated; }
    size_t Capacity() const { return m_Array.size(); }
    void ClearAllocation() { m_Allocated = 0; }
    void MemZero() { memset(this, 0, sizeof(*this)); }
    void MemZeroAndClear() { ClearAllocation(); MemZero();  }

    template<typename T>
    void fill(T && t) { return m_Array.fill(std::forward<T>(t)); }

    iterator begin() { return m_Array.begin(); }
    const_iterator begin() const { return m_Array.begin(); }
    iterator end() { return m_Array.begin() + m_Allocated; }
    const_iterator end() const { return m_Array.begin() + m_Allocated; }

    bool DeallocateLast() { 
        if (m_Allocated == 0)
            return false;
        --m_Allocated; 
        return true;
    }
    bool Allocate(size_t &index) {
        if (m_Allocated >= SIZE)
            return false;
        index = m_Allocated++;
        return true;
    }

    ELEMENT *Allocate() {
        if (m_Allocated >= SIZE)
            return nullptr;
        auto index = m_Allocated;
        m_Allocated++;
        return &(*this)[index];
    }

    template<typename T>
    bool push(T && t) {
        if (m_Allocated >= SIZE)
            return false;
        auto index = m_Allocated;
        m_Allocated++;
        m_Array[index] = std::forward<T>(t);
        return true;
    }

    template<typename T>
    bool append_copy(T t[], size_t cnt) {
        if (m_Allocated + cnt >= SIZE)
            return false;
        for (size_t i = 0; i < cnt; ++i) {
            m_Array[m_Allocated + i] = t[i];
        }
        m_Allocated += cnt;
        return true;
    }

    template<typename T>
    ELEMENT pop(T && t) {
        if (m_Allocated == 0)
            return std::forward<T>(t);
        --m_Allocated;
        auto index = m_Allocated;
        return m_Array[index];
    }						  

    bool empty() const { return m_Allocated == 0; }

    reference operator[](size_type _Pos) {
        return m_Array[_Pos];
    }

    constexpr const_reference operator[](size_type _Pos) const {
        return m_Array[_Pos];
    }
protected:
    size_t m_Allocated;
    ArrayType m_Array;
};

}
