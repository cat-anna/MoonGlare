#pragma once

#include <array>
#include <cstdint>
#include <fmt/format.h>
#include <orbit_logger.h>

namespace MoonGlare::Memory {

//TODO: add unit tests for BasicElementPool

template <class ElementType_t, size_t Capacity_t, typename IndexType_t = size_t>
struct BasicElementPool {

    using IndexType = IndexType_t;
    using ElementType = ElementType_t;
    static constexpr size_t kCapacity = Capacity_t;

    using ArrayType = std::array<ElementType, kCapacity>;

    BasicElementPool() : allocated(0), valid(0) {}

    IndexType Allocated() const { return allocated; }
    IndexType ValidElements() const { return valid; }

    constexpr IndexType Capacity() const { return storage.size(); }
    IndexType Reserve() const { return Capacity() - ValidElements(); }

    ElementType *Storage() { return &storage[0]; }

    void ClearAllocation() { allocated = 0; }
    bool Empty() const { return allocated == 0; }
    void InvalidateStoredElements() { valid = 0; }

    template <typename T>
    void Fill(T &&t) {
        return storage.fill(std::forward<T>(t));
    }

    bool Insert(ElementType item) {
        if (valid >= Capacity()) {
            return false;
        }
        auto next = valid++;
        storage[next] = std::move(item);
        return true;
    }

    ElementType *BulkInsert(IndexType count) {
        if (valid + count > Capacity()) {
            return nullptr;
        }
        auto next = valid;
        valid += count;
        return &storage[next];
    }

    bool Allocate(ElementType &item) {
        if (allocated >= valid) {
            return false;
        }
        auto next = allocated++;
        item = storage[next];
        return true;
    }

protected:
    IndexType allocated;
    IndexType valid;
    ArrayType storage;
};

template <class ElementType_t, size_t Capacity_t, typename IndexType_t = size_t,
          ElementType_t InvalidHandleValue_t = ElementType_t(0)>
struct HandleElementPool : public BasicElementPool<ElementType_t, Capacity_t, IndexType_t> {

    using BaseType = BasicElementPool<ElementType_t, Capacity_t, IndexType_t>;
    using BaseType::ElementType;

    static constexpr ElementType kInvalidHandleValue = InvalidHandleValue_t;

    ElementType Next() {
        ElementType value = kInvalidHandleValue;
        if (!this->Allocate(value)) {
            AddLog(Info,
                   fmt::format("{}: handle resource exhausted. Returning invalid handle value.",
                               typeid(*this).name()));
        }
        return value;
    }
};

} // namespace MoonGlare::Memory
