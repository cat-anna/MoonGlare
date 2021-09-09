#pragma once

#include <array>
#include <atomic>
#include <cstring>
#include <string>
#include <vector>

namespace MoonGlare::Memory {

//TODO: add unit tests for StackAllocator

//-----------------------------------------------------------------------------

template <typename Storage_t, bool EnforceAlignment16 = false>
struct BasicStackAllocator {
    BasicStackAllocator() : allocated(0) {}

    BasicStackAllocator(BasicStackAllocator &&) = delete;
    BasicStackAllocator(const BasicStackAllocator &) = delete;

    using ItemType = uint8_t;
    using Storage = Storage_t;

    bool Empty() const { return allocated == 0; }

    size_t Allocated() const { return allocated; }

    void PartialClear(size_t alloc) { allocated = alloc; }
    void ClearAllocation() { allocated = 0; }

    const char *CloneString(const std::string &str) {
        return Clone<char>(str.c_str(), str.size() + 1);
    }
    const char *CloneString(const char *data) { return Clone<char>(data, strlen(data) + 1); }

    template <typename T>
    T *Clone(const T *data, size_t Count) {
        T *ptr = Allocate<T>(Count);
        if (ptr != nullptr) {
            memcpy(ptr, data, Count * sizeof(T));
        }
        return ptr;
    }

    template <typename T>
    T *Clone(const std::vector<T> &data) {
        T *ptr = Allocate<T>(data.size());
        if (ptr != nullptr) {
            memcpy(ptr, &data[0], data.size() * sizeof(T));
        }
        return ptr;
    }

    template <typename T, size_t COUNT>
    T *Clone(const std::array<T, COUNT> &data) {
        T *ptr = Allocate<T>(data.size());
        if (ptr != nullptr) {
            memcpy(ptr, &data[0], data.size() * sizeof(T));
        }
        return ptr;
    }

    template <typename T, size_t COUNT>
    T *Clone(const T (&data)[COUNT]) {
        T *ptr = Allocate<T>(COUNT);
        if (ptr != nullptr) {
            memcpy(ptr, &data[0], COUNT * sizeof(T));
        }
        return ptr;
    }

    template <class T>
    T *Allocate(size_t Count = 1) {
        if (Count == 0) {
            return nullptr;
        }

        size_t byte_size = sizeof(T) * Count;
        if constexpr (EnforceAlignment16) {
            byte_size = Align16(byte_size);
        }

        size_t position = allocated;
        if (!CanContain(byte_size)) {
            return nullptr;
        }
        allocated = allocated + byte_size;
        return reinterpret_cast<T *>(static_cast<Storage *>(this)->GetMemory() + position);
    }

protected:
    size_t allocated;

    bool CanContain(size_t bytes) const {
        return static_cast<const Storage *>(this)->HasReserveFor(bytes);
    }
};

//-----------------------------------------------------------------------------

template <size_t _capacity, bool EnforceAlignment16 = false>
struct StaticStackAllocator
    : public BasicStackAllocator<StaticStackAllocator<_capacity, EnforceAlignment16>,
                                 EnforceAlignment16> {
public:
    static constexpr size_t kCapacity = _capacity;

    size_t Capacity() const { return kCapacity; }
    size_t Reserve() const { return Capacity() - this->Allocated(); }
    bool HasReserveFor(size_t bytes) const { return this->Allocated() + bytes < Capacity(); }

    const uint8_t *GetMemory() const { return &memory[0]; }
    uint8_t *GetMemory() { return &memory[0]; }

    void Zero() {
        this->ClearAllocation();
        memory.fill(0);
    }

private:
    std::array<uint8_t, kCapacity> memory;
};

//-----------------------------------------------------------------------------

} // namespace MoonGlare::Memory
