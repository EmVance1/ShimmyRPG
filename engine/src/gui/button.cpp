#include "pch.h"
#include "button.h"


namespace gui {

Button::Button(const Position& position, const sf::Vector2f& size, const Style& style, const std::string& label)
    : TextWidget(position, size, style)
{
    set_label(label);
}


bool Button::handle_event(const sf::Event& event) {
    TextWidget::handle_event(event);
    bool clicked = false;
    if (event.is<sf::Event::MouseButtonPressed>()) {
        if (is_hovered()) {
            clicked = true;
            if (m_has_callback) {
                m_callback();
            }
        }
    }
    if (clicked) {
        set_background_color(get_style().background_color_3);
    } else if (is_hovered()) {
        set_background_color(get_style().background_color_2);
    } else {
        set_background_color(get_style().background_color_1);
    }
    return is_hovered();
}

}
