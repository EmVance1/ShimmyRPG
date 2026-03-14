#include "pch.h"
#include "gui/button.h"


namespace gui {

Button::Button(const Position& position, const Sizing& sizing, const Style& style, const std::string& label)
    : TextWidget(position, sizing, style)
{
    set_label(label);
}


bool Button::handle_event(const sf::Event& event) {
    TextWidget::handle_event(event);
    if (event.is<sf::Event::MouseButtonPressed>()) {
        m_clicked = is_hovered();
        if (m_clicked && m_has_callback) {
            m_callback();
        }
    } else if (event.is<sf::Event::MouseButtonReleased>()) {
        // if (is_hovered() && m_clicked && m_has_callback) {
        //     m_callback();
        // }
        m_clicked = false;
    } else if (!event.is<sf::Event::MouseMoved>()) {
        return false;
    }
    if (m_clicked) {
        set_background_color(get_style_variant().bg_3);
    } else if (is_hovered()) {
        set_background_color(get_style_variant().bg_2);
    } else {
        set_background_color(get_style_variant().bg_1);
    }
    return is_hovered();
}

}
