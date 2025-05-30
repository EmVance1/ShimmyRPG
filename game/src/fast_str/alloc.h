#pragma once
#include <cstdlib>
#include <type_traits>
#include <stdexcept>
#include <utility>
#include <vector>
#include <functional>


#ifdef _MSC_VER
    #define aligned_malloc(size, alignment) _aligned_malloc(size, alignment)
    #define aligned_free(ptr) _aligned_free(ptr)
#else
    #define aligned_malloc(size, alignment) std::aligned_alloc(alignment, size)
    #define aligned_free(ptr) std::free(ptr)
#endif


template<typename T>
class Allocator {
public:
    virtual T* alloc() = 0;
    virtual T* alloc_arr(size_t count) = 0;
    virtual T* realloc_arr(T* val, size_t before, size_t after) = 0;
    virtual void dealloc(T* val) = 0;
    virtual void dealloc_arr(T* val, size_t count) = 0;
};

template<typename T>
class Malloc : public Allocator<T> {
public:
    static_assert(std::is_trivial_v<T>, "T must be a trivial type");

    Malloc() = default;

    T* alloc() override {
        void* raw = aligned_malloc(sizeof(T), alignof(T));
        if (!raw) { throw std::bad_alloc(); }
        return static_cast<T*>(raw); 
    }
    T* alloc_arr(size_t count) override {
        void* raw = aligned_malloc(sizeof(T) * count, alignof(T));
        if (!raw) { throw std::bad_alloc(); }
        return static_cast<T*>(raw);
    }
    T* realloc_arr(T* val, size_t, size_t after) override {
        return (T*)std::realloc(val, after * sizeof(T));
    }
    void dealloc(T* val) override {
        aligned_free(val);
    }
    void dealloc_arr(T* val, size_t) override {
        aligned_free(val);
    }

    static Malloc<T> Inst;
};

template<typename T>
Malloc<T> Malloc<T>::Inst = Malloc<T>{};


template<typename T>
class NewAlloc : public Allocator<T> {
public:
    NewAlloc() = default;

    T* alloc() override {
        return new T();
    }
    T* alloc_arr(size_t count) override {
        return new T[count];
    }
    T* realloc_arr(T* val, size_t before, size_t after) override {
        static_assert(std::is_move_constructible_v<T> || std::is_copy_constructible_v<T>, "T must be move-constructible or copy-constructible");
        T* temp = new T[after];
        for (size_t i = 0; i < std::min(before, after); ++i) {
            temp[i] = std::move_if_noexcept(val[i]);
        }
        delete[] val;
        return temp;
    }
    void dealloc(T* val) override {
        delete val;
    }
    void dealloc_arr(T* val, size_t) override {
        delete[] val;
    }

    static NewAlloc<T> Inst;
};

template<typename T>
NewAlloc<T> NewAlloc<T>::Inst = NewAlloc<T>{};


template<typename T>
struct Arena {
    T* block;
    size_t capacity;

    Arena(size_t count) : block(new T[count]), capacity(count) {}
    ~Arena() { delete[] block; }
};

template<typename T>
class ArenaAlloc : public Allocator<T> {
private:
    Arena<T>* m_arena;
    T* m_ptr;

public:
    ArenaAlloc(Arena<T>* arena) : m_arena(arena), m_ptr(m_arena->block) {}

    template<typename ... Args>
    T* alloc(Args&& ... args) {
        T* temp = m_ptr;
        m_ptr++;
        if (m_ptr - m_arena->block > m_arena->capacity) { throw std::bad_alloc(); }
        return new(temp)T(std::forward<Args>(args)...);
    }
    T* alloc() override {
        T* temp = m_ptr;
        m_ptr++;
        if (m_ptr - m_arena->block > m_arena->capacity) { throw std::bad_alloc(); }
        return temp;
    }
    T* alloc_arr(size_t count) override {
        T* temp = m_ptr;
        m_ptr += count;
        if (m_ptr - m_arena->block > m_arena->capacity) { throw std::bad_alloc(); }
        return temp;
    }
    T* realloc_arr(T* val, size_t before, size_t after) override {
        static_assert(std::is_move_constructible_v<T> || std::is_copy_constructible_v<T>, "T must be move-constructible or copy-constructible");
        T* temp = alloc_arr(after);
        for (size_t i = 0; i < std::min(before, after); i++) {
            temp[i] = std::move_if_noexcept(val[i]);
        }
        return temp;
    }
    void dealloc(T*) override {}
    void dealloc_arr(T*, size_t) override {}
};


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
    T* alloc(const T& val) {
        T* temp = alloc_aligned(sizeof(T), alignof(T));
        if constexpr (std::is_trivial_v<T>) {
            *temp = val;
        } else {
            register_dtor<T>(temp);
            new(temp)T(val);
        }
        return temp;
    }
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

