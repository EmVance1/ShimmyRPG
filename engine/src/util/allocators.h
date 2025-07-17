#pragma once
#include <cstdlib>
#include <stdexcept>
#include <utility>
#include <vector>


struct Destructor {
    void (*call)(void*);
    void* obj;
    void operator()() const { call(obj); }
};


class GenericArenaAlloc {
private:
    uint8_t* m_arena;
    uint8_t* m_ptr;
    size_t m_cap;
    std::vector<Destructor> m_reg;

private:
    void* alloc_aligned(size_t size, size_t alignment) {
        size_t rem = m_cap - (m_ptr - m_arena);
        void* ptr = static_cast<void*>(m_ptr);
        if (!std::align(alignment, size, ptr, rem)) {
            throw std::bad_alloc();
        }
        m_ptr = static_cast<uint8_t*>(ptr) + size;
        if ((size_t)(m_ptr - m_arena) > m_cap) { throw std::bad_alloc(); }
        return ptr;
    }

    template<typename T>
    void register_dtor(T* ptr, size_t count = 1) {
        if constexpr (!std::is_trivially_destructible_v<T>) {
            for (size_t i = 0; i < count; ++i) {
                m_reg.emplace_back({
                    [](void* p) { static_cast<T*>(p)->~T(); },
                    ptr + i,
                });
            }
        }
    }

public:
    GenericArenaAlloc(size_t size)
        : m_arena(static_cast<uint8_t*>(std::calloc(size, sizeof(char)))), m_ptr(m_arena), m_cap(size)
    {
        if (!m_arena) { throw std::bad_alloc(); }
    }
    ~GenericArenaAlloc() {
        reset();
        std::free(m_arena);
    }

    void reset() {
        for (int i = (int)m_reg.size() - 1; i >= 0; i--) {
            m_reg[i]();
        }
        m_ptr = m_arena;
    }

    template<typename T>
    T* alloc() {
        T* temp = alloc_aligned(sizeof(T), alignof(T));
        if constexpr (!std::is_trivial_v<T>) {
            register_dtor<T>(temp);
            new(temp)T();
        }
        return temp;
    }
    template<typename T>
    T* clone(const T& val) {
        T* temp = alloc_aligned(sizeof(T), alignof(T));
        if constexpr (std::is_trivial_v<T>) {
            *temp = val;
        } else {
            register_dtor<T>(temp);
            new(temp)T(val);
        }
        return temp;
    }
    /*
    template<typename T>
    T* alloc(T&& val) {
        T* temp = alloc_aligned(sizeof(T), alignof(T));
        if constexpr (std::is_trivial_v<T>) {
            *temp = val;
        } else {
            register_dtor<T>(temp);
            new(temp)T(std::move_if_noexcept(val));
        }
        return temp;
    }
    */
    template<typename T, typename ... Args>
    T* alloc_emplace(Args&& ... args) {
        T* temp = alloc_aligned(sizeof(T), alignof(T));
        register_dtor<T>(temp);
        return new(temp)T(std::forward<Args>(args)...);
    }

    template<typename T>
    T* alloc_arr(size_t count) {
        T* temp = alloc_aligned(sizeof(T) * count, alignof(T));
        if constexpr (!std::is_trivial_v<T>) {
            register_dtor<T>(temp, count);
            for (size_t i = 0; i < count; i++) {
                new(temp + i)T();
            }
        }
        return temp;
    }
    template<typename T>
    T* alloc_arr(size_t count, const T& val) {
        T* temp = alloc_aligned(sizeof(T) * count, alignof(T));
        if constexpr (std::is_trivial_v<T>) {
            for (size_t i = 0; i < count; i++) {
                temp[i] = val;
            }
        } else {
            register_dtor<T>(temp, count);
            for (size_t i = 0; i < count; i++) {
                new(temp + i)T(val);
            }
        }
        return temp;
    }
    template<typename T, typename ... Args>
    T* alloc_arr_emplace(size_t count, Args&& ... args) {
        T* temp = alloc_aligned(sizeof(T) * count, alignof(T));
        register_dtor<T>(temp, count);
        for (size_t i = 0; i < count; i++) {
            new(temp + i)T(std::forward<Args>(args)...);
        }
        return temp;
    }

    // provided for API compliance, does nothing
    template<typename T>
    void dealloc(T*) {}
    // provided for API compliance, does nothing
    template<typename T>
    void dealloc_arr(T*, size_t) {}
};




template<typename T>
class PoolAllocator {
private:
    T* m_pool;
    bool* m_list;
    size_t m_cap;

public:
    PoolAllocator(size_t n)
        : m_pool((T*)malloc(sizeof(T) * n)), m_list((bool*)calloc(n, sizeof(bool))), m_cap(n)
    {}
    PoolAllocator(const PoolAllocator&) = delete;
    PoolAllocator(PoolAllocator&& other)
        : m_pool(other.m_pool), m_list(other.m_list), m_cap(other.m_cap)
    {
        other.m_pool = nullptr;
        other.m_list = nullptr;
    }
    ~PoolAllocator() {
        dealloc_all();
        free(m_pool);
        free(m_list);
    }

    void dealloc_all() {
        for (size_t i = 0; i < m_cap; i++) {
            if (m_list[i]) {
                m_list[i] = false;
                m_pool[i].~T();
            }
        }
    }

    void reset(size_t n) {
        dealloc_all();
        free(m_pool);
        free(m_list);
        m_pool = (T*)malloc(sizeof(T) * n);
        m_list = (bool*)calloc(n, sizeof(bool));
        m_cap = n;
    }

    T* alloc() {
        size_t idx = 0;
        for (; idx < m_cap && m_list[idx]; idx++);
        if (idx == m_cap) { throw std::bad_alloc(); }
        m_list[idx] = true;
        new(m_pool+idx)T();
        return m_pool + idx;
    }
    T* alloc(const T& val) {
        size_t idx = 0;
        for (; idx < m_cap && m_list[idx]; idx++);
        if (idx == m_cap) { throw std::bad_alloc(); }
        m_list[idx] = true;
        new(m_pool+idx)T(val);
        return m_pool + idx;
    }
    T* alloc(T&& val) {
        size_t idx = 0;
        for (; idx < m_cap && m_list[idx]; idx++);
        if (idx == m_cap) { throw std::bad_alloc(); }
        m_list[idx] = true;
        new(m_pool+idx)T(std::move_if_noexcept(val));
        return m_pool + idx;
    }
    template<typename ...Args>
    T* alloc(Args&& ...args) {
        size_t idx = 0;
        for (; idx < m_cap && m_list[idx]; idx++);
        if (idx == m_cap) { throw std::bad_alloc(); }
        m_list[idx] = true;
        new(m_pool+idx)T(std::forward<Args>(args)...);
        return m_pool + idx;
    }

    void dealloc(const T* obj) {
        const size_t idx = obj - m_pool;
        m_list[idx] = false;
        m_pool[idx].~T();
    }
};

