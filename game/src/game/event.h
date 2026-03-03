#pragma once
#include <SFML/Graphics.hpp>
#include <deque>


namespace shmy {

class Event;

class EventQueue {
private:
    struct TypeRegistry {
        inline static size_t counter = 0;

        template<typename T>
        static size_t id() {
            static size_t id = counter++;
            return id;
        }
    };
    template<typename D> friend class EventBase;

private:
    std::deque<std::unique_ptr<Event>> m_impl;

public:
    void push_event(std::unique_ptr<Event>&& event);
    std::optional<std::unique_ptr<Event>> poll_event();
};

class Event {
public:
    virtual ~Event() = default;
    virtual size_t type() const = 0;
};
template<typename D>
class EventBase : Event {
public:
    static size_t type_id() {
        return EventQueue::TypeRegistry::id<D>();
    }
    size_t type() const override {
        return type_id();
    }
};

}
