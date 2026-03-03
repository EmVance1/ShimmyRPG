#include "pch.h"
#include "event.h"


namespace shmy {

std::optional<std::unique_ptr<Event>> EventQueue::poll_event() {
    if (m_impl.empty()) return {};
    auto val = std::move(m_impl.front());
    m_impl.pop_front();
    return val;
}

void EventQueue::push_event(std::unique_ptr<Event>&& event) {
    m_impl.push_back(std::move(event));
}

}
