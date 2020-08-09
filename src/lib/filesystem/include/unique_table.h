#pragma once

#include <memory>

namespace MoonGlare::StarVfs {

struct unique_table_construction_helper {
    template <class T> static void create(std::unique_ptr<T[]> &t, size_t size);
};

template <class T> inline void unique_table_construction_helper::create(std::unique_ptr<T[]> &t, size_t size) {
    t.reset(new T[size]);
}

template <> inline void unique_table_construction_helper::create(std::unique_ptr<char[]> &t, size_t size) {
    t.reset(new char[size + 1]);
    t[size] = 0;
}

template <> inline void unique_table_construction_helper::create(std::unique_ptr<int8_t[]> &t, size_t size) {
    t.reset(new int8_t[size + 1]);
    t[size] = 0;
}

template <> inline void unique_table_construction_helper::create(std::unique_ptr<uint8_t[]> &t, size_t size) {
    t.reset(new uint8_t[size + 1]);
    t[size] = 0;
}

template <class T> struct unique_table {
    using item_t = T;

    unique_table(const unique_table &) = delete;
    unique_table(unique_table &&) = default;
    unique_table &operator=(const unique_table &) = delete;
    unique_table &operator=(unique_table &&) = default;
    unique_table() : m_Table(), m_Size(0) {}

    void make_new(size_t len) {
        unique_table_construction_helper::create(m_Table, len);
        m_Size = len;
    }

    bool empty() const { return m_Size == 0; }

    /// CAUTION: use this function very carefully!!
    template <class OTHER> void make_copy(const OTHER *other, size_t count) {
        if (!other || !count) {
            reset();
            return;
        }
        size_t bytes = count * sizeof(OTHER);
        make_new(bytes / sizeof(T));
        memcpy(m_Table.get(), other, byte_size());
    }
    /// CAUTION: use this function very carefully!!
    template <class OTHER> void make_copy(const unique_table<OTHER> &other) {
        if (!other) {
            reset();
            return;
        }
        size_t bytes = other.byte_size();
        make_new(bytes / sizeof(T));
        memcpy(m_Table.get(), other.get(), byte_size());
    }
    /// CAUTION: use this function very carefully!!
    template <class OTHER> void assign_from(unique_table<OTHER> &other) {
        if (!other) {
            reset();
            return;
        }
        m_Size = other.byte_size() / sizeof(T);
        m_Table.reset((T *)other.release());
    }

    void from_string(const std::string &str) {
        make_new(str.length());
        memcpy(m_Table.get(), str.data(), byte_size());
    }
    void copy_from(const void *data, size_t length) {
        make_new(length);
        memcpy(m_Table.get(), data, byte_size());
    }

    T *get() { return m_Table.get(); }
    const T *get() const { return m_Table.get(); }
    const char *c_str() const { return (const char *)get(); }

    size_t size() const { return m_Size; }
    size_t byte_size() const { return m_Size * sizeof(T); }
    static size_t element_size() { return sizeof(T); }

    T &operator[](size_t idx) {
        StarVFSAssert(idx < m_Size);
        return m_Table[idx];
    }
    const T &operator[](size_t idx) const {
        StarVFSAssert(idx < m_Size);
        return m_Table[idx];
    }

    void swap(unique_table &other) {
        if (this == &other)
            return;
        other.m_Table.swap(m_Table);
        std::swap(other.m_Size, m_Size);
    }

    void reset() {
        m_Size = 0;
        m_Table.reset();
    }

    void reset(T *memory, size_t size) {
        m_Size = size;
        m_Table.reset(memory);
    }

    /// hide some of real size of table
    void trim_table(size_t s) {
        assert(s <= m_Size);
        m_Size = s;
    }

    T *release() {
        m_Size = 0;
        return m_Table.release();
    }

    struct base_iterator {
        base_iterator() : m_Ptr(nullptr) {}
        explicit base_iterator(T *p) : m_Ptr(p) {}

        bool operator<(const base_iterator &other) const { return m_Ptr < other.m_Ptr; }
        bool operator<=(const base_iterator &other) const { return m_Ptr <= other.m_Ptr; }
        bool operator>(const base_iterator &other) const { return m_Ptr > other.m_Ptr; }
        bool operator>=(const base_iterator &other) const { return m_Ptr >= other.m_Ptr; }
        bool operator==(const base_iterator &other) const { return m_Ptr == other.m_Ptr; }
        bool operator!=(const base_iterator &other) const { return m_Ptr != other.m_Ptr; }

        const T &operator*() const { return *m_Ptr; }
        const T *operator->() const { return m_Ptr; }

    protected:
        T *m_Ptr;
    };

    struct const_iterator : public base_iterator {
        const_iterator() : base_iterator(nullptr) {}
        explicit const_iterator(T *p) : const_iterator(p) {}
        const_iterator &operator++() {
            StarVFSAssert(this->m_Ptr);
            ++this->m_Ptr;
            return *this;
        }
        const_iterator operator++(int) {
            StarVFSAssert(this->m_Ptr);
            return const_iterator(this->m_Ptr + 1);
        }
        const_iterator &operator--() {
            StarVFSAssert(this->m_Ptr);
            --this->m_Ptr;
            return *this;
        }
        const_iterator operator--(int) {
            StarVFSAssert(this->m_Ptr);
            return const_iterator(this->m_Ptr - 1);
        }
    };

    struct iterator : public base_iterator {
        iterator() : base_iterator(nullptr) {}
        explicit iterator(T *p) : base_iterator(p) {}
        iterator &operator++() {
            StarVFSAssert(this->m_Ptr);
            ++this->m_Ptr;
            return *this;
        }
        iterator operator++(int) {
            StarVFSAssert(this->m_Ptr);
            return iterator(this->m_Ptr + 1);
        }
        iterator &operator--() {
            StarVFSAssert(this->m_Ptr);
            --this->m_Ptr;
            return *this;
        }
        iterator operator--(int) {
            StarVFSAssert(this->m_Ptr);
            return iterator(this->m_Ptr - 1);
        }

        T &operator*() { return *this->m_Ptr; }
        T *operator->() { return this->m_Ptr; }
    };

    iterator begin() {
        if (!m_Table)
            return iterator(nullptr);
        return iterator(m_Table.get());
    }
    const_iterator begin() const {
        if (!m_Table)
            return const_iterator(nullptr);
        return const_iterator(m_Table.get());
    }
    iterator end() {
        if (!m_Table)
            return iterator(nullptr);
        return iterator(m_Table.get() + m_Size);
    }
    const_iterator end() const {
        if (!m_Table)
            return const_iterator(nullptr);
        return const_iterator(m_Table.get() + m_Size);
    }

    operator bool() const { return static_cast<bool>(m_Table); }

    void memset(char value) {
        if (m_Table && m_Size)
            memset(m_Table.get(), value, byte_size());
    }

private:
    std::unique_ptr<T[]> m_Table;
    size_t m_Size = 0;
};

} // namespace MoonGlare::StarVfs
