#include "pch.h"
#include "uuid.h"
#include "container.h"


namespace gui {

Container::Container(const Position& position, const sf::Vector2f& size, const Style& style)
    : Widget(position, size, style)
{}


void Container::add_widget(const std::string& name, const std::shared_ptr<Widget>& widget, bool inherit_style) {
    widget->set_container(this);
    widget->set_style_inherited(inherit_style);
    if (inherit_style) {
        widget->set_style(get_style());
    }
    m_children[name] = widget;
}

void Container::add_widget(const std::shared_ptr<Widget>& widget, bool inherit_style) {
    add_widget(Uuid::generate_v4(), widget, inherit_style);
}


std::shared_ptr<Widget> Container::get_widget(const std::string& id) {
    return m_children[id];
}

std::shared_ptr<const Widget> Container::get_widget(const std::string& id) const {
    return m_children.at(id);
}

bool Container::has_widget(const std::string& id) const {
    return m_children.find(id) != m_children.end();
}

std::shared_ptr<Widget> Container::remove_widget(const std::string& id) {
    auto temp = m_children[id];
    m_children.erase(id);
    return temp;
}



void Container::set_position(const Position& position) {
    Widget::set_position(position);
    for (auto& [_, w] : m_children) {
        w->set_position(w->get_position());
    }
}

void Container::set_style(const Style& style) {
    Widget::set_style(style);
    for (auto& [_, w] : m_children) {
        if (w->is_style_inherited()) {
            w->set_style(style);
        }
    }
}


void Container::update() {
    auto kill_list = std::vector<std::string>();
    for (auto& [k, w] : m_children) {
        if (w->is_enabled()) {
            w->update();
        }
        if (w->is_destroyed()) {
            kill_list.push_back(k);
        }
    }
    for (const auto& k : kill_list) {
        m_children.erase(k);
    }
}

bool Container::handle_event(const sf::Event& event) {
    bool hovered = false;
    for (auto& [_, w] : m_children) {
        if (w->is_enabled()) {
            if (w->handle_event(event)) {
                hovered = true;
            }
        }
    }
    return hovered;
}

void Container::draw(sf::RenderTarget& target, sf::RenderStates states) const {
    auto sorted = std::vector<std::shared_ptr<Widget>>();
    for (const auto& [k, w] : m_children) {
        if (w->is_visible()) {
            sorted.push_back(w);
        }
    }
    std::sort(sorted.begin(), sorted.end(), [](const auto& a, const auto& b){ return a->get_sorting_layer() < b->get_sorting_layer(); });
    Widget::draw(target, states);
    states.transform *= get_transform();
    for (const auto& w : sorted) {
        target.draw(*w, states);
    }
}

}
