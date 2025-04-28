#include <cstdlib>
#include <stdexcept>


class ArenaAllocator {
private:
    void* m_block;
    void* m_ptr;
    size_t m_cap = 0;

public:
    template<typename T>
    ArenaAllocator(size_t objects) {
        m_block = malloc(objects * sizeof(T));
        m_ptr = m_block;
        m_cap = objects * sizeof(T);
    }
    ArenaAllocator(size_t bytes) {
        m_block = malloc(bytes);
        m_ptr = m_block;
        m_cap = bytes;
    }
    ArenaAllocator(const ArenaAllocator&) = delete;
    ~ArenaAllocator() {
        free(m_block);
    }

    template<typename T>
    void reinit(size_t objects) {
        free(m_block);
        m_block = malloc(objects * sizeof(T));
        m_ptr = m_block;
        m_cap = objects;
    }
    void reinit(size_t bytes) {
        free(m_block);
        m_block = malloc(bytes);
        m_ptr = m_block;
        m_cap = bytes;
    }

    void* alloc(size_t bytes) {
        const auto temp = m_ptr;
        m_ptr = (char*)m_ptr + bytes;
        return temp;
    }
    template<typename T>
    T* alloc() {
        const T* temp = m_ptr;
        m_ptr = (char*)m_ptr + sizeof(T);
        return temp;
    }
};


template<typename T>
class StackAllocator {
private:
    T* m_begin;
    T* m_ptr;
    size_t m_cap = 0;

public:
    StackAllocator(size_t objects) {
        m_begin = new T[objects];
        m_ptr = m_begin;
        m_cap = objects;
    }
    StackAllocator(const StackAllocator&) = delete;
    ~StackAllocator() {
        delete[] m_begin;
    }

    void reinit(size_t objects) {
        delete[] m_begin;
        m_begin = new T[objects];
        m_ptr = m_begin;
        m_cap = objects;
    }

    T* begin() { return m_begin; }
    T* end()   { return m_ptr; }
    T* top()   { return m_ptr - 1; }

    T* push() {
        const T* temp = m_ptr;
        *temp = T();
        m_ptr++;
        return temp;
    }
    void pop() {
        m_ptr--;
    }
};


template<typename T>
class FreeListAllocator {
private:
    T* m_block;
    bool* m_freelist;
    size_t m_cap;

public:
    FreeListAllocator(size_t n)
        : m_block(new T[n]), m_freelist(new bool[n]), m_cap(n)
    {
        for (size_t i = 0; i < n; i++) {
            m_freelist[i] = true;
        }
    }
    FreeListAllocator(const FreeListAllocator&) = delete;
    ~FreeListAllocator() {
        delete[] m_block;
        delete[] m_freelist;
    }

    void free_all() {
        for (size_t i = 0; i < m_cap; i++) {
            m_freelist[i] = true;
        }
    }

    void reset(size_t n) {
        delete[] m_block;
        delete[] m_freelist;
        m_block = new T[n];
        m_freelist = new bool[n];
        m_cap = n;
        for (size_t i = 0; i < n; i++) {
            m_freelist[i] = true;
        }
    }

    T* alloc() {
        for (size_t i = 0; i < m_cap; i++) {
            if (m_freelist[i]) {
                m_freelist[i] = false;
                return m_block + i;
            }
        }
        throw std::exception("allocator out of space");
    }
    T* alloc(const T& val) {
        for (size_t i = 0; i < m_cap; i++) {
            if (m_freelist[i]) {
                m_freelist[i] = false;
                m_block[i] = val;
                return m_block + i;
            }
        }
        throw std::exception("allocator out of space");
    }
    T* alloc(T&& val) {
        for (size_t i = 0; i < m_cap; i++) {
            if (m_freelist[i]) {
                m_freelist[i] = false;
                m_block[i] = std::move(val);
                return m_block + i;
            }
        }
        throw std::exception("allocator out of space");
    }
    // template<typename ...Args>
    // T* alloc(Args&& ...args) {
    //     for (size_t i = 0; i < m_cap; i++) {
    //         if (m_freelist[i]) {
    //             m_freelist[i] = false;
    //             new(m_block + i) T(std::forward<Args>(args)...);
    //             return m_block + i;
    //         }
    //     }
    //     throw std::exception("allocator out of space");
    // }

    void free(const T* obj) {
        const size_t idx = obj - m_block;
        m_freelist[idx] = true;
    }
};

