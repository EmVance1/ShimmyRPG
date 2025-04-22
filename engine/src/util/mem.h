#pragma once
#include <optional>
#include <mutex>


template<typename T>
class LockGuard {
private:
    std::mutex& m_impl;
    T* p_resource;

private:
    LockGuard(std::mutex& mutex, T* resource) : m_impl(mutex), p_resource(resource) {}

public:
    ~LockGuard() {
        release();
    }

    void release() {
        p_resource = nullptr;
        m_impl.unlock();
    }

    T& operator*() {
        return *p_resource;
    }
    const T& operator*() const {
        return *p_resource;
    }

    T* operator->() {
        return p_resource;
    }
    const T* operator->() const {
        return p_resource;
    }

    template<typename U>
    friend class Mutex;
};


template<typename T>
class Mutex {
private:
    std::mutex m_impl;
    T* m_resource;

public:
    Mutex(T& resource) : m_resource(&resource) {}

    LockGuard<T> lock() {
        m_impl.lock();
        return LockGuard<T>(m_impl, m_resource);
    }

    std::optional<LockGuard<T>> try_lock() {
        if (m_impl.try_lock()) {
            return LockGuard<T>(m_resource);
        } else {
            return {};
        }
    }
};

