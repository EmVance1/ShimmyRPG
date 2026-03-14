#pragma once
#include <SFML/Graphics.hpp>
#include "../style.h"
#include "widget.h"


namespace gui {

class TextWidget : public Widget {
private:
    std::vector<sf::Text> m_labels;
    std::u32string m_label_value;
    uint32_t m_character_size = 22;
    float m_text_padding = 0.f;
    float m_line_spacing = 0.f;
    float m_char_spacing = 0.f;
    Position m_textpos = Position{sf::Vector2f(0, 0)};

protected:
    void update_textalign();
    void update_textpos();
    virtual void update_transform() override;

public:
    TextWidget(const Position& position, const Sizing& sizing, const Style& style);

    virtual void set_style(const Style& style) override;
    virtual void set_style_variant(size_t variant) override;

    void set_character_size(uint32_t character_size);
    uint32_t get_character_size() const { return m_character_size; }

    void set_text_position(Position position);
    Position get_text_position() const { return m_textpos; }

    void set_text_padding(float padding);
    float get_text_padding() const { return m_text_padding; }

    void set_text_line_spacing(float spacing);
    float get_text_line_spacing() const { return m_line_spacing; }

    void set_text_char_spacing(float spacing);
    float get_text_char_spacing() const { return m_char_spacing; }

    void set_label(const std::string& label);
    void set_label(const std::u32string& label);
    const std::u32string& get_label() const { return m_label_value; }

    virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const override;
};

}
