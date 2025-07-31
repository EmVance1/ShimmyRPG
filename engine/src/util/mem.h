#pragma once
#include <mutex>
#include <shared_mutex>
#include <optional>


template<typename T>
class LockGuard {
private:
    std::unique_lock<std::mutex> m_impl;
    T* p_resource;

private:
    LockGuard(std::mutex& mutex, T* resource) : m_impl(mutex), p_resource(resource) {}

public:
    T* get() { return p_resource; }
    const T* get() const { return p_resource; }

    T& operator*() { return *p_resource; }
    const T& operator*() const { return *p_resource; }

    T* operator->() { return p_resource; }
    const T* operator->() const { return p_resource; }

    template<typename U>
    friend class Mutex;
};


template<typename T>
class Mutex {
private:
    std::mutex m_impl;
    T m_resource;

public:
    Mutex(T&& resource) : m_resource(std::move(resource)) {}
    Mutex(const T& resource) : m_resource(resource) {}

    LockGuard<T> acquire() {
        return LockGuard<T>(m_impl, &m_resource);
    }
};


template<typename T>
class ReadLockGuard {
private:
    std::shared_lock<std::shared_mutex> m_impl;
    const T* p_resource;

private:
    ReadLockGuard(std::shared_mutex& impl, const T* resource) : m_impl(impl), p_resource(resource) {}

public:
    const T* get() const { return p_resource; }
    const T& operator*() const { return *p_resource; }
    const T* operator->() const { return p_resource; }

    template<typename U>
    friend class RWMutex;
};


template<typename T>
class WriteLockGuard {
private:
    std::unique_lock<std::shared_mutex> m_impl;
    T* p_resource;

private:
    WriteLockGuard(std::shared_mutex& impl, T* resource) : m_impl(impl), p_resource(resource) {}

public:
    T* get() { return p_resource; }
    const T* get() const { return p_resource; }
    T& operator*() { return *p_resource; }
    const T& operator*() const { return *p_resource; }
    T* operator->() { return p_resource; }
    const T* operator->() const { return p_resource; }

    template<typename U>
    friend class RWMutex;
};


template<typename T>
class RWMutex {
private:
    std::shared_mutex m_lock;
    T m_resource;

public:
    RWMutex(T&& resource) : m_resource(std::move(resource)) {}
    RWMutex(const T& resource) : m_resource(resource) {}

    ReadLockGuard<T> acquire_read() {
        return ReadLockGuard<T>(m_lock, &m_resource);
    }
    WriteLockGuard<T> acquire_write() {
        return WriteLockGuard<T>(m_lock, &m_resource);
    }
};

