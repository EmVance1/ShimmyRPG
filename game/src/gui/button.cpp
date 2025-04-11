#include "pch.h"
#include "button.h"


namespace gui {

Button::Button(const Position& position, const sf::Vector2f& size, const Style& style, const std::string& label)
    : Widget(position, size, style), m_label(style.font), m_value(label)
{
    m_shape.setSize(size);
    m_shape.setFillColor(m_style->background_color_1);
    m_shape.setOutlineColor(m_style->outline_color_1);
    m_shape.setOutlineThickness(m_style->outline_width);

    m_label.setPosition(sf::Vector2f(10.f, 0.f));
    m_label.setCharacterSize((uint32_t)(size.y * 0.8f));
    m_label.setFillColor(m_style->text_color_1);
    m_label.setString(label);
}


void Button::update() {
    if (m_clicked) {
        m_shape.setFillColor(m_style->background_color_3);
    } else if (is_hovered()) {
        m_shape.setFillColor(m_style->background_color_2);
    } else {
        m_shape.setFillColor(m_style->background_color_1);
    }
}

bool Button::handle_event(const sf::Event& event) {
    Widget::handle_event(event);
    m_clicked = false;
    if (event.is<sf::Event::MouseButtonPressed>()) {
        if (is_hovered()) {
            m_clicked = true;
            if (m_has_callback) {
                m_callback();
            }
            return true;
        }
    }
    return false;
}

void Button::draw(sf::RenderTarget& target, sf::RenderStates states) const {
    states.transform *= get_transform();
    target.draw(m_shape, states);
    target.draw(m_label, states);
}

}
