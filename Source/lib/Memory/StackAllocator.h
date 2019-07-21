#pragma once

#include <atomic>

#include "Memory.h"
#include "LockPolicy.h"

namespace MoonGlare::Memory {

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

template<template<class> class ALLOCATOR>
struct StackAllocator {
    template<typename ... ARGS>
    StackAllocator(ARGS ... args) : m_Allocated(0), m_Storage(std::forward<ARGS>(args)...) {}

    //not supported yet:
    //copy ctor
    //move ctor
    //copy operator
    //move operator

    using Item_t = char;
    using Storage_t = ALLOCATOR<Item_t>;

    bool Empty() const {
        return m_Allocated == 0;
    }
    size_t Reserve() const {
        return Capacity() - Allocated();
    }
    size_t Capacity() const {
        return m_Storage.ElementCount();
    }
    size_t Allocated() const {
        return m_Allocated;
    }
    void PartialClear(size_t alloc) {
        m_Allocated = alloc;
    }
    void Clear() {
        m_Allocated = 0;
    }

    const char* CloneString(const std::string &str) {
        char* ptr = Allocate<char>(str.length() + 1);
        if (ptr) {
            memcpy(ptr, str.c_str(), str.length());
            ptr[str.length()] = '\0';
        }
        return ptr;
    }

    const char* CloneString(const char* data) {
        auto len = strlen(data);
        char* ptr = Allocate<char>(len + 1);
        if (ptr) {
            memcpy(ptr, data, len);
            ptr[len] = '\0';
        }
        return ptr;
    }

    template <typename T>
    T* Clone(const T* data, size_t Count) {
        T* ptr = Allocate<T>(Count);
        if (ptr)
            memcpy(ptr, data, Count * sizeof(T));
        return ptr;
    }

    template <typename T>
    T* Clone(const std::vector<T>& data) {
        T* ptr = Allocate<T>(data.size());
        if(ptr)
            memcpy(ptr, &data[0], data.size() * sizeof(T));
        return ptr;
    }

    template <typename T, size_t COUNT>
    T* Clone(const std::array<T, COUNT>& data) {
        T* ptr = Allocate<T>(data.size());
        if (ptr)
            memcpy(ptr, &data[0], data.size() * sizeof(T));
        return ptr;
    }

    template <typename T, size_t COUNT>
    T* Clone(const T (&data)[COUNT]) {
        T* ptr = Allocate<T>(COUNT);
        if (ptr)
            memcpy(ptr, &data[0], COUNT * sizeof(T));
        return ptr;
    }

    template <class T>
    T* Allocate(size_t Count = 1) {
        //static_assert (std::is_pod<T>::value, "Not a pod type!");
        if (Count == 0)
            return nullptr;

        size_t ByteSize = sizeof(T) * Count;
        size_t allocated = m_Allocated;
        if (allocated + ByteSize >= Capacity()) {
            return nullptr;
        }
        m_Allocated = allocated + ByteSize;
        return reinterpret_cast<T*>(m_Storage.GetMemory() + allocated);
    }

    template <class T>
    T* AllocateAligned(size_t Count = 1) {
        //static_assert (std::is_pod<T>::value, "Not a pod type!");
        if (Count == 0)
            return nullptr;

        size_t ByteSize = sizeof(T) * Count;

        size_t allocated = m_Allocated;
        void *storage = m_Storage.GetMemory() + allocated;

        size_t offset = (size_t) ((uintptr_t) storage & (alignof(T) - 1));
        if (offset > 0)
            offset = alignof(T) - offset;

        ByteSize += offset;
        if (allocated + ByteSize >= Capacity()) {
            return nullptr;
        }

        m_Allocated = allocated + ByteSize;

        return reinterpret_cast<T*>((char*)storage + offset);
    }

    Storage_t& GetStorage() {
        return m_Storage;
    }
    const Storage_t& GetStorage() const {
        return m_Storage;
    }
protected:
    size_t m_Allocated;
    Storage_t m_Storage;
};

//-----------------------------------------------------------------------------

}
