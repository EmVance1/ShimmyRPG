#pragma once
#include <SFML/Graphics.hpp>
#include "../style.h"
#include "widget.h"


namespace gui {

class TextWidget : public Widget {
private:
    sf::Text m_label;
    std::string m_label_value = "";
    uint32_t m_character_size = 25;
    Alignment m_text_alignment = Alignment::TopLeft;
    float m_text_padding = 5.f;

protected:
    void apply_text_alignment();

public:
    TextWidget(const Position& position, const sf::Vector2f& size, const Style& style)
        : Widget(position, size, style), m_label(style.font)
    {
        m_label.setCharacterSize(m_character_size);
        m_label.setFillColor(style.text_color_1);
        apply_text_alignment();
    }

    void set_character_size(uint32_t character_size) { m_character_size = character_size; }
    uint32_t get_character_size() const { return m_character_size; }

    void set_text_alignment(Alignment alignment) { m_text_alignment = alignment; apply_text_alignment(); }
    Alignment get_text_alignment() const { return m_text_alignment; }

    void set_text_padding(float padding) { m_text_padding = padding; apply_text_alignment(); }
    float get_text_padding() const { return m_text_padding; }

    void set_label(const std::string& label) { m_label_value = label; m_label.setString(label); apply_text_alignment(); }
    const std::string& get_label() const { return m_label_value; }

    virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const {
        Widget::draw(target, states);
        states.transform *= get_transform();
        target.draw(m_label, states);
    }
};

}
