#pragma once

#include <array>

namespace MoonGlare::Memory {

template<size_t SIZE>
struct DynamicBuffer {
    static constexpr size_t BufferSize = SIZE;
    using ElementSize = uint32_t;
    using ArrayType = std::array<uint8_t, BufferSize>;

    DynamicBuffer() { }
    void Zero() { allocatedBytes = 0; buffer.fill(0); }
    void Clear() { allocatedBytes = 0; buffer[0] = 0; }
    bool Empty() const { return allocatedBytes == 0; }

    enum class Magic : uint32_t {
        Value = 'tnve',
    };

    struct EntryHeader {
        Magic magic;
        ElementSize size;

    };

    template<typename T>
    struct Entry : public EntryHeader {
        T t;
    };

    template<typename T>
    T* Allocate() {
        using E = Entry<T>;
        static_assert(std::is_trivial_v<T>);
        if (allocatedBytes + sizeof(E) >= BufferSize)
            return nullptr;       
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

    struct EndIterator {
        EndIterator(const size_t &pos, const DynamicBuffer& db) : position(pos), bufref(db) { }
        size_t Limit() const { return position; }
    private:
        const size_t &position;
        const DynamicBuffer &bufref;
    };
    struct Iterator {
        Iterator(size_t pos, const DynamicBuffer& db) : position(pos), bufref(db) { }
        bool operator != (const Iterator &other) const { return position != other.position; }
        bool operator != (const EndIterator &other) const { return position != other.Limit(); }
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
    EndIterator end() const { return EndIterator(allocatedBytes, *this); }
private:
    size_t allocatedBytes;
    ArrayType buffer;
};
                                     
}
