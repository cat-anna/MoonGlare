#pragma once

#include "AllocatorCommon.h"
#include <array>

namespace MoonGlare::Memory {

template <class CHAR, size_t SIZE, class LOCKPOLICY = DefaultLockPolicy> struct StaticStringPool {
    using Char_t = CHAR;
    using LockPolicy = LOCKPOLICY;
    enum {
        Size = SIZE,
    };

    size_t RemainSize() const { return Size - m_UsedSize; }
    size_t RemainBytes() const { return RemainSize() * sizeof(Char_t); }
    size_t UsedSize() const { return m_UsedSize; }
    size_t UsedBytes() const { return UsedSize() * sizeof(Char_t); }
    void Clear() { m_UsedSize = 0; }

    StaticStringPool() : m_UsedSize(0) {}

    /** allocs space for len chars*/
    Char_t *Allocate(size_t len) {
        ++len;
        Char_t *ptr;
        {
            LockPolicy::Guard_t lock(m_Lock);
            if (len > RemainSize())
                return nullptr;
            ptr = m_Pool + m_UsedSize;
            m_UsedSize += len;
        }
        ptr[len] = static_cast<Char_t>(0);
        ptr[len - 1] = static_cast<Char_t>(0);
        return ptr;
    }

    Char_t *Allocate(const Char_t *str) {
        size_t len = 0;
        if (!str)
            return Allocate((size_t)0);
        while (str[len])
            ++len;
        return Allocate(str, len);
    }

    Char_t *Allocate(const Char_t *str, size_t len) {
        auto *ptr = Allocate(len);
        if (ptr)
            memcpy(ptr, str, sizeof(Char_t) * len);
        return ptr;
    }

private:
    LockPolicy m_Lock;
    size_t m_UsedSize;
    Char_t m_Pool[Size];
};

//----------------------------------------------------------------

template <class T, size_t SIZE, class LOCKPOLICY = DefaultLockPolicy> struct StaticMemoryPool {
    static_assert(std::is_pod<T>::value, "StaticMemoryPool accepts only POD types");

    using Item = T;
    using LockPolicy = LOCKPOLICY;
    enum {
        Size = SIZE,
    };

    template <class Iter_T> struct BaseIterator {
        using Item = Iter_T;
        using ThisClass = BaseIterator<Iter_T>;
        ThisClass(Item *ptr) : m_Ptr(ptr) {}
        Item &operator*() { return *m_Ptr; }
        std::add_const_t<Item> &operator*() const { return *m_Ptr; }
        ThisClass operator++(int) { return ThisClass(m_Ptr + 1); }
        ThisClass &operator++() {
            ++m_Ptr;
            return *this;
        }
        ThisClass operator--(int) { return ThisClass(m_Ptr - 1); }
        ThisClass &operator--() {
            ++m_Ptr;
            return *this;
        }
        bool operator!=(const ThisClass &other) const { return m_Ptr != other.m_Ptr; }
        bool operator>(const ThisClass &other) const { return m_Ptr > other.m_Ptr; }
        bool operator<(const ThisClass &other) const { return m_Ptr < other.m_Ptr; }

    private:
        Item *m_Ptr;
    };

    using iterator = BaseIterator<T>;
    using const_iterator = BaseIterator<const T>;

    StaticMemoryPool() : m_UsedSize(0) {}

    size_t RemainSize() const { return Size - m_UsedSize; }
    size_t RemainBytes() const { return RemainSize() * sizeof(Char_t); }
    size_t UsedSize() const { return m_UsedSize; }
    size_t UsedBytes() const { return UsedSize() * sizeof(Char_t); }
    void Clear() { m_UsedSize = 0; }
    bool Empty() const { return m_UsedSize == 0; }

    /** Allocate single element */
    T *Allocate() { return Allocate(1); }
    /** Allocate single element and memset to zero */
    T *AllocateAndZero() { return AllocateAndZero(1); }
    /** Allocate multilple elements  */
    T *Allocate(size_t len) {
        if (RemainSize() < len)
            return nullptr;
        LockPolicy::Guard_t lock(m_Lock);
        T *ptr = m_Memory + m_UsedSize;
        m_UsedSize += len;
        return ptr;
    }
    /** Allocate multilple elements and memset to zero */
    T *AllocateAndZero(size_t len) {
        auto ptr = Allocate(len);
        if (ptr)
            memset(ptr, 0, sizeof(T) * len);
        return ptr;
    }

    iterator begin() { return iterator(m_Memory); }
    const_iterator begin() const { return const_iterator(m_Memory); }
    iterator end() { return iterator(m_Memory + m_UsedSize); }
    const_iterator end() const { return const_iterator(m_Memory + m_UsedSize); }

private:
    LockPolicy m_Lock;
    size_t m_UsedSize;
    T m_Memory[Size];
};

template <class TYPE, size_t SIZE> struct BaseStaticMemory {
    using IsPOD = typename std::is_pod<TYPE>::type;

    constexpr bool Realloc(size_t /*NewSize*/) const noexcept { return false; }

    using Item_t = TYPE;
    using Resizable = std::false_type;

    constexpr size_t ElementCount() const noexcept { return SIZE; }
    constexpr size_t ElementSize() const noexcept { return sizeof(TYPE); }
    constexpr size_t ByteSize() const noexcept { return ElementSize() * ElementCount(); }
};

//-----------------------------------------------------------------------------

template <class TYPE, size_t SIZE> struct StaticPointerMemory : public BaseStaticMemory<TYPE, SIZE> {
    using Item_t = typename BaseStaticMemory<TYPE, SIZE>::Item_t;

    StaticPointerMemory() { m_Ptr.reset(new Item_t[SIZE]); }

    ~StaticPointerMemory() { m_Ptr.reset(); }

    // copy ctor
    // move ctor
    // copy operator
    // move operator

    void swap(StaticPointerMemory &other) {
        LIBSPACE_ASSERT(&other != this);
        std::swap(m_Ptr, other.m_Ptr);
    }

    Item_t *GetMemory() noexcept { return m_Ptr.get(); }
    const Item_t *GetMemory() const noexcept { return m_Ptr.get(); }

protected:
    std::unique_ptr<Item_t[]> m_Ptr;
};

//-----------------------------------------------------------------------------

template <class TYPE> struct StaticTableMemory {
    using IsPOD = typename std::is_pod<TYPE>::type;
    using Item_t = TYPE;
    using Resizable = std::false_type;

    StaticTableMemory(Item_t *ptr, uint32_t size) {
        m_Ptr = ptr;
        m_Size = size;
    }

    ~StaticTableMemory() {}

    constexpr bool Realloc(size_t /*NewSize*/) const noexcept { return false; }
    constexpr size_t ElementSize() const noexcept { return sizeof(TYPE); }

    size_t ElementCount() const noexcept { return m_Size; }
    size_t ByteSize() const noexcept { return ElementSize() * ElementCount(); }

    Item_t *GetMemory() noexcept { return m_Ptr; }
    const Item_t *GetMemory() const noexcept { return m_Ptr; }

protected:
    uint32_t m_Size;
    Item_t *m_Ptr;
};

//-----------------------------------------------------------------------------

template <class TYPE, size_t SIZE> struct StaticMemory : public BaseStaticMemory<TYPE, SIZE> {
    StaticMemory() {}
    ~StaticMemory() {}

    using Item_t = typename BaseStaticMemory<TYPE, SIZE>::Item_t;

    // copy ctor
    // move ctor
    // copy operator
    // move operator
    // swap function

    void Fill(const TYPE &value = TYPE()) noexcept(std::is_nothrow_copy_assignable<TYPE>::value) {
        m_Memory.fill(value);
    }

    void MemZero() noexcept { memset(GetMemory(), 0, this->ByteSize()); }

    Item_t *GetMemory() noexcept { return &m_Memory[0]; }
    const Item_t *GetMemory() const noexcept { return &m_Memory[0]; }

protected:
    std::array<Item_t, SIZE> m_Memory;
};

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

template <class TYPE, class POINTERTYPE> struct BaseDynamicAllocator {
    BaseDynamicAllocator() : m_Size(0) {}
    ~BaseDynamicAllocator() {}

    using IsPOD = typename std::is_pod<TYPE>::type;

    constexpr bool Realloc(size_t /*NewSize*/) const noexcept { return false; }

    using Resizable = std::true_type;

    using Item_t = TYPE;
    using Pointer_t = POINTERTYPE; //

    void swap(BaseDynamicAllocator &other) {
        LIBSPACE_ASSERT(&other != this);
        std::swap(m_Ptr, other.m_Ptr);
        std::swap(m_Size, other.m_Size);
    }

    size_t ElementCount() const noexcept { return m_Size; }
    size_t ByteSize() const noexcept { return ElementSize() * ElementSize(); }
    constexpr size_t ElementSize() const noexcept { return sizeof(TYPE); }

    Item_t *GetMemory() noexcept { return m_Ptr.get(); }
    const Item_t *GetMemory() const noexcept { return m_Ptr.get(); }

protected:
    size_t m_Size;
    Pointer_t m_Ptr;
};

//-----------------------------------------------------------------------------

template <class TYPE>
struct DynamicPointerAllocator_POD : public BaseDynamicAllocator<TYPE, std::unique_ptr<TYPE[], FreeDeallocator>> {
    DynamicPointerAllocator_POD() {}
    ~DynamicPointerAllocator_POD() {}

    using BaseClass = BaseDynamicAllocator<TYPE, std::unique_ptr<TYPE[], FreeDeallocator>>;
    using Item_t = typename BaseClass::Item_t;
    using Pointer_t = typename BaseClass::Pointer_t;

    // copy ctor
    // move ctor
    // copy operator
    // move operator

    void MemZero() noexcept {
        if (this->ElementCount() > 0)
            memset(this->GetMemory(), 0, this->ByteSize());
    }

    bool Realloc(size_t NewSize) noexcept {
        if (this->ElementCount() == NewSize)
            return true;
        if (NewSize == 0)
            this->m_Ptr.reset();
        else {
            void *mem = this->GetMemory();
            this->m_Ptr.release();
            this->m_Ptr.reset(static_cast<Item_t *>(::realloc(mem, this->ElementSize() * NewSize)));
        }
        this->m_Size = NewSize;
        return true;
    }
};

//-----------------------------------------------------------------------------

template <class TYPE>
struct DynamicPointerAllocator_NOPOD : public BaseDynamicAllocator<TYPE, std::unique_ptr<TYPE[]>> {
    DynamicPointerAllocator_NOPOD() {}
    ~DynamicPointerAllocator_NOPOD() {}

    using BaseClass = BaseDynamicAllocator<TYPE, std::unique_ptr<TYPE[]>>;
    using Item_t = typename BaseClass::Item_t;
    using Pointer_t = typename BaseClass::Pointer_t;

    // copy ctor
    // move ctor
    // copy operator
    // move operator

    bool Realloc(size_t NewSize) { // cannot noexcept
        if (this->ElementCount() == NewSize)
            return true;
        if (NewSize == 0)
            this->m_Ptr.reset();
        else {
            Pointer_t newptr = Pointer_t(new TYPE[NewSize]);

            size_t j = this->ElementCount() < NewSize ? this->ElementCount() : NewSize;
            for (size_t i = 0; i < j; ++i)
                std::swap(newptr[i], this->m_Ptr[i]);

            this->m_Ptr.swap(newptr);
        }
        this->m_Size = NewSize;
        return true;
    }
};

//-----------------------------------------------------------------------------

template <class TYPE>
struct DynamicExternalPointerAllocator_POD
    : public BaseDynamicAllocator<TYPE, std::unique_ptr<TYPE[], NullDeallocator>> {
    DynamicExternalPointerAllocator_POD() {}
    ~DynamicExternalPointerAllocator_POD() {}

    using Resizable = std::false_type;
    using BaseClass = BaseDynamicAllocator<TYPE, std::unique_ptr<TYPE[], NullDeallocator>>;
    using Item_t = typename BaseClass::Item_t;
    using Pointer_t = typename BaseClass::Pointer_t;

    // copy ctor
    // move ctor
    // copy operator
    // move operator

    void MemZero() noexcept {
        if (this->ElementCount() > 0)
            memset(this->GetMemory(), 0, this->ByteSize());
    }

    void SetMemory(TYPE *memory, size_t Capacity) {
        this->m_Ptr.reset(memory);
        this->m_Size = Capacity;
    }
    void ReleaseMemory() { SetMemory(nullptr, 0); }

    bool Realloc(size_t /*NewSize*/) noexcept { return false; }
};

//-----------------------------------------------------------------------------

template <class TYPE>
struct DynamicExternalPointerAllocator_NOPOD
    : public BaseDynamicAllocator<TYPE, std::unique_ptr<TYPE[], NullDeallocator>> {
    DynamicExternalPointerAllocator_NOPOD() {}
    ~DynamicExternalPointerAllocator_NOPOD() {}

    using Resizable = std::false_type;
    using BaseClass = BaseDynamicAllocator<TYPE, std::unique_ptr<TYPE[], NullDeallocator>>;
    using Item_t = typename BaseClass::Item_t;
    using Pointer_t = typename BaseClass::Pointer_t;

    // copy ctor
    // move ctor
    // copy operator
    // move operator

    void SetMemory(TYPE *memory, size_t Capacity) {
        this->m_Ptr.reset(memory);
        this->m_Size = Capacity;
    }
    void ReleaseMemory() { SetMemory(nullptr, 0); }

    bool Realloc(size_t /*NewSize*/) noexcept { return false; }
};

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

template <class TYPE, class STORAGE> struct AllocatorInterface : public STORAGE {
    using BaseClass = STORAGE;
    using ThisClass = AllocatorInterface<TYPE, STORAGE>;
    using Item_t = TYPE;

    AllocatorInterface(){};
    ~AllocatorInterface(){};

    AllocatorInterface(const AllocatorInterface &) = delete;
    AllocatorInterface(AllocatorInterface &&) = delete;
    AllocatorInterface &operator=(const AllocatorInterface &) = delete;
    AllocatorInterface &operator=(AllocatorInterface &&) = delete;

    // not supported yet:
    // copy ctor
    // move ctor
    // copy operator
    // move operator

    void Fill(const TYPE &value = TYPE()) noexcept(std::is_nothrow_copy_assignable<TYPE>::value) {
        TYPE *memory = this->GetMemory();
        for (size_t i = 0; i < this->ElementCount(); ++i)
            memory[i] = value;
    }

    Item_t &operator[](size_t index) {
        LIBSPACE_BOUNDARY_ASSERT(index < this->ElementCount());
        return *(this->GetMemory() + index);
    }
    const Item_t &operator[](size_t index) const {
        LIBSPACE_BOUNDARY_ASSERT(index < this->ElementCount());
        return *(this->GetMemory() + index);
    }
};

} // namespace MoonGlare::Memory
