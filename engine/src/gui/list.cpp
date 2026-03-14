#include "pch.h"
#include "gui/list.h"


namespace gui {

VerticalList::VerticalList(const Position& position, const Sizing& sizing, const Style& style)
    : Container(position, sizing, style)
{}


void VerticalList::add_widget(const std::string& name, const std::shared_ptr<Widget>& widget, bool top) {
    float size = 0.f;
    for (const auto& [k, v] : m_children) {
        size += v->get_absolute_size().y;
    }
    Container::add_widget(name, widget, top);
    widget->set_position(Position({ 0.f, size }));
    set_sizing({ get_sizing().x_size, lo::absolute(size + widget->get_absolute_size().y) });
    m_order.push_back(name);
}

std::string VerticalList::add_widget(const std::shared_ptr<Widget>& widget, bool top) {
    float size = 0.f;
    for (const auto& [k, v] : m_children) {
        size += v->get_absolute_size().y;
    }
    const auto id = Container::add_widget(widget, top);
    widget->set_position(Position({ 0.f, size }));
    set_sizing({ get_sizing().x_size, lo::absolute(size + widget->get_absolute_size().y) });
    m_order.push_back(id);
    return id;
}

std::shared_ptr<Widget> VerticalList::remove_widget(const std::string& id) {
    if (has_widget(id)) {
        auto w = Container::remove_widget(id);
        m_order.erase(std::find(m_order.begin(), m_order.end(), id));
        return w;
    } else {
        return nullptr;
    }
}

void VerticalList::clear() {
    Container::clear();
    set_sizing({ get_sizing().x_size, lo::absolute(0.f) });
}

void VerticalList::refresh() {
    float size = 0.f;
    for (auto w : m_order) {
        get_widget(w)->set_position(Position({ 0.f, size }));
        size += get_widget(w)->get_absolute_size().y;
    }
    set_sizing({ get_sizing().x_size, lo::absolute(size) });
}

}
