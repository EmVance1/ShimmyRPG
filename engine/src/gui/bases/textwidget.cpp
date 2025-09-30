#include "pch.h"
#include <utf8.h>
#include "gui/bases/textwidget.h"


namespace gui {

static std::u32string wrapped(const std::u32string& src, const sf::Font& font, uint32_t character_size, float width) {
    auto result = std::u32string();
    auto word = std::u32string();
    auto line_width = 0.f;
    auto word_width = 0.f;
    auto last = U'\0';

    for (const char32_t c : src) {
        const auto glyph = font.getGlyph(c, character_size, false);
        const auto diff = glyph.advance + font.getKerning(last, c, character_size);
        if (line_width + word_width + diff <= width) {
            word.push_back(c);
            word_width += diff;
            if (c == ' ' || c == '\t' || c == '\n' || c == '\r') {
                result.insert(result.end(), word.begin(), word.end());
                word.clear();
                line_width += word_width;
                word_width = 0.f;
            }
        } else if (word_width + diff <= width) {
            result.push_back('\n');
            line_width = 0.f;
            word.push_back(c);
            word_width += diff;
            if (c == U' ' || c == U'\t' || c == U'\n' || c == U'\r') {
                result.insert(result.end(), word.begin(), word.end());
                word.clear();
                line_width += word_width;
                word_width = 0.f;
            }
        } else {
            result.insert(result.end(), word.begin(), word.end());
            word.clear();
            line_width = glyph.advance;
            word_width = 0.f;
            result.push_back(U'\n');
            result.push_back(c);
        }
        last = c;
    }

    if (line_width + word_width <= width) { // * 1.125f) {
        result.insert(result.end(), word.begin(), word.end());
    } else {
        result.push_back(U'\n');
        result.insert(result.end(), word.begin(), word.end());
    }

    return result;
}

void TextWidget::apply_text_alignment() {
    m_label.setString(wrapped(m_label_value, get_style().font, get_character_size(), get_bounds().size.x - 2.f * m_text_padding));

    const auto c_size = get_size();
    const auto t_size = m_label.getLocalBounds().size;
    switch (m_text_alignment) {
    case Position::Alignment::TopLeft:
        m_label.setPosition({m_text_padding, m_text_padding}); break;
    case Position::Alignment::CenterLeft:
        m_label.setPosition({m_text_padding, c_size.y * 0.5f - t_size.y * 0.5f}); break;
    case Position::Alignment::BottomLeft:
        m_label.setPosition({m_text_padding, c_size.y - t_size.y - m_text_padding}); break;
    case Position::Alignment::TopCenter:
        m_label.setPosition({c_size.x * 0.5f - t_size.x * 0.5f, m_text_padding}); break;
    case Position::Alignment::Center:
        m_label.setPosition(c_size * 0.5f - t_size * 0.5f); break;
    case Position::Alignment::BottomCenter:
        m_label.setPosition({c_size.x * 0.5f - t_size.x * 0.5f, c_size.y - t_size.y - m_text_padding}); break;
    case Position::Alignment::TopRight:
        m_label.setPosition({c_size.x - t_size.x - m_text_padding, m_text_padding}); break;
    case Position::Alignment::CenterRight:
        m_label.setPosition({c_size.x - t_size.x - m_text_padding, c_size.y * 0.5f - t_size.y * 0.5f}); break;
    case Position::Alignment::BottomRight:
        m_label.setPosition({c_size.x - t_size.x - m_text_padding, c_size.y - t_size.y - m_text_padding}); break;
    }
    m_label.move(sf::Vector2f(0.f, -5.f));
}


void TextWidget::set_character_size(uint32_t character_size) {
    m_character_size = character_size;
    m_label.setCharacterSize(character_size);
    apply_text_alignment();
}

void TextWidget::set_text_alignment(Position::Alignment alignment) {
    m_text_alignment = alignment;
    apply_text_alignment();
}

void TextWidget::set_text_padding(float padding) {
    m_text_padding = padding;
    apply_text_alignment();
}

void TextWidget::set_text_color(const sf::Color& color) {
    m_label.setFillColor(color);
}

void TextWidget::set_label(const std::string& label) {
    m_label_value = utf8::utf8to32(label);
    m_label.setString(label);
    apply_text_alignment();
}

void TextWidget::set_label(const std::u32string& label) {
    m_label_value = label;
    m_label.setString(label);
    apply_text_alignment();
}


}

