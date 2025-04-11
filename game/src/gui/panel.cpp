#include "pch.h"
#include "panel.h"


namespace gui {

void Panel::add_widget(const std::string& name, const std::shared_ptr<Widget>& widget, bool inherit_style) {
    widget->set_container(this);
    widget->set_style_inherited(inherit_style);
    if (inherit_style) {
        widget->set_style(get_style());
    }
    m_widgets[name] = widget;
}

std::shared_ptr<Widget> Panel::get_widget(const std::string& id) {
    return m_widgets[id];
}

std::shared_ptr<const Widget> Panel::get_widget(const std::string& id) const {
    return m_widgets.at(id);
}

bool Panel::has_widget(const std::string& id) const {
    return m_widgets.find(id) != m_widgets.end();
}

std::shared_ptr<Widget> Panel::remove_widget(const std::string& id) {
    auto temp = m_widgets[id];
    m_widgets.erase(id);
    return temp;
}



void Panel::set_position(const Position& position) {
    const auto diff = position.offset - Widget::get_absolute_position();
    Widget::set_position(position);
    for (auto& [_, w] : m_widgets) {
        w->set_position(w->get_position().moved(diff));
    }
}

void Panel::set_style(const Style& style) {
    Widget::set_style(style);
    for (auto& [_, w] : m_widgets) {
        if (w->is_style_inherited()) {
            w->set_style(style);
        }
    }
}


void Panel::update() {
    for (auto& [_, w] : m_widgets) {
        if (w->is_enabled()) {
            w->update();
        }
    }
}

bool Panel::handle_event(const sf::Event& event) {
    Widget::handle_event(event);
    bool changed = false;
    for (auto& [_, w] : m_widgets) {
        if (w->is_enabled()) {
            changed |= w->handle_event(event);
        }
    }
    return changed;
}

void Panel::draw(sf::RenderTarget& target, sf::RenderStates states) const {
    states.transform *= get_transform();
    for (const auto& [_, w] : m_widgets) {
        if (w->is_visible()) {
            target.draw(*w, states);
        }
    }
}

}
