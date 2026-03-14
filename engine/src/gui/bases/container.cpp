#include "pch.h"
#include "util/uuid.h"
#include "gui/bases/container.h"


namespace gui {

Container::Container(const Position& position, const Sizing& sizing, const Style& style)
    : Widget(position, sizing, style)
{}


void Container::update_transform() {
    Widget::update_transform();
    for (auto& [_, w] : m_children) {
        w->update_transform();
    }
}


void Container::set_style(const Style& style) {
    Widget::set_style(style);
    for (auto& [_, w] : m_children) {
        w->set_style(style);
    }
}

void Container::set_style_variant(size_t variant) {
    Widget::set_style_variant(variant);
    for (auto& [_, w] : m_children) {
        w->set_style_variant(variant);
    }
}


void Container::add_widget(const std::string& name, const std::shared_ptr<Widget>& widget, bool top) {
    m_children[name] = widget;
    widget->set_style(get_style());
    widget->set_container(this);
    if (top) {
        int layer = 0;
        for (const auto& [_, w] : m_children) {
            if (w->get_sorting_layer() > layer) {
                layer = w->get_sorting_layer();
            }
        }
        widget->set_sorting_layer(layer + 1);
    }
}

std::string Container::add_widget(const std::shared_ptr<Widget>& widget, bool top) {
    const auto id = shmy::core::generate_uuid_v4();
    add_widget(id, widget, top);
    return id;
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
    bool consumed = false;
    for (auto& [_, w] : m_children) {
        if (w->is_enabled()) {
            if (w->handle_event(event)) {
                consumed = true;
            }
        }
    }
    return consumed;
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
    for (const auto& w : sorted) {
        target.draw(*w, states);
    }
}

}
