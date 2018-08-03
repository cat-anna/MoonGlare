#pragma once

#include <array>

namespace MoonGlare::Memory {

template<size_t SIZE>
struct DynamicBuffer {
    static constexpr size_t BufferSize = SIZE;
    using ElementSize = uint16_t;
    using ArrayType = std::array<uint8_t, BufferSize>;

    DynamicBuffer() { }
    void Zero() { allocatedBytes = 0; buffer.fill(0); }
    void Clear() { allocatedBytes = 0; buffer[0] = 0; }
    bool Empty() const { return allocatedBytes == 0; }

    enum class Magic : uint16_t {
        Value = 0xFADE,
    };

    struct alignas(1) EntryHeader {
        Magic magic;
        ElementSize size;
    };

    template<typename T>
    struct alignas(1) Entry : public EntryHeader {
        T t;
    };

    template<typename T>
    T* Allocate() {
        static_assert(std::is_trivial_v<T>);
        if (allocatedBytes + sizeof(T) >= BufferSize)
            return nullptr;       
        using E = Entry<T>;
        E* ptr = reinterpret_cast<E*>(&buffer[0] + allocatedBytes);
        ptr->magic = Magic::Value;
        ptr->size = sizeof(E);
        allocatedBytes += sizeof(E);
        buffer[allocatedBytes] = 0;
        return &ptr->t;
    }

    struct Element {
        ElementSize size;
        const void *memory;
    };

    struct Iterator {
        Iterator(size_t pos, const DynamicBuffer& db) : position(pos), bufref(db) { }
        bool operator != (const Iterator &other) const {
            return position != other.position;
        }
        void operator++() {
            position += GetEntry()->size;
        }
        const Element operator*() const {
            auto *entry = GetEntry();
            size_t d = sizeof(EntryHeader);
            Element e;
            e.size = entry->size;
            e.memory = reinterpret_cast<const uint8_t*>(entry) + d;
            return e;
        }
    private:
        size_t position;
        const DynamicBuffer &bufref;

        const EntryHeader *GetEntry() const {
            const auto *ptr = &bufref.buffer[0] + position;
            const EntryHeader *be = reinterpret_cast<const EntryHeader*>(ptr);
            assert(be->magic == Magic::Value);
            if (be->magic == Magic::Value)
                return be;

            __debugbreak();
            return nullptr;
        }
    };

    Iterator begin() const { return Iterator(0, *this); }
    Iterator end() const { return Iterator(allocatedBytes, *this); }
private:
    size_t allocatedBytes;
    ArrayType buffer;
};
                                     
}
