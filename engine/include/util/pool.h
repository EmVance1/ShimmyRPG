#include <stdexcept>


namespace shmy {

template<typename T>
class PoolAllocator {
private:
    T* m_pool;
    bool* m_list;
    size_t m_cap;

public:
    PoolAllocator(size_t n)
        : m_pool((T*)malloc(n * sizeof(T))), m_list((bool*)calloc(n, sizeof(bool))), m_cap(n)
    {}
    PoolAllocator(const PoolAllocator&) = delete;
    PoolAllocator(PoolAllocator&& other) noexcept
        : m_pool(other.m_pool), m_list(other.m_list), m_cap(other.m_cap)
    {
        other.m_pool = nullptr;
        other.m_list = nullptr;
    }
    ~PoolAllocator() {
        clear();
        free(m_pool);
        free(m_list);
    }

    void clear() {
        for (size_t i = 0; i < m_cap; i++) {
            if (m_list[i]) {
                m_list[i] = false;
                m_pool[i].~T();
            }
        }
    }

    void reset(size_t n) {
        clear();
        free(m_pool);
        free(m_list);
        m_pool = (T*)malloc(n * sizeof(T));
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

}
