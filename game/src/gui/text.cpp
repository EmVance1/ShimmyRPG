#include "pch.h"
#include "text.h"


namespace gui {

Text::Text(const Position& position, uint32_t character_size, const Style& style, const std::string& label)
    : Widget(position, sf::Vector2f(0.f, (float)character_size * 1.2f), style), m_label(style.font), m_value(label)
{
    m_label.setCharacterSize(character_size);
    m_label.setFillColor(m_style->text_color_1);
    m_label.setString(label);

    m_bounds.size.x = m_label.getGlobalBounds().size.x;
    update_position();

    m_shape.setPosition(sf::Vector2f(-10.f, 0.f));
    m_shape.setSize(m_bounds.size + sf::Vector2f(20.f, 0.f));
    m_shape.setFillColor(m_style->background_color_1);
    m_shape.setOutlineColor(m_style->outline_color_1);
    m_shape.setOutlineThickness(m_style->outline_width);
}


void Text::draw(sf::RenderTarget& target, sf::RenderStates states) const {
    states.transform *= get_transform();
    if (m_background_enabled) {
        target.draw(m_shape, states);
    }
    target.draw(m_label, states);
}

}
