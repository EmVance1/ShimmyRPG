#include "pch.h"
#include "textwidget.h"


namespace gui {

static void extend_str(std::string& sink, const std::string& src) {
    for (const auto& c : src) {
        sink.push_back(c);
    }
}

static std::string wrapped(const std::string& src, const sf::Font& font, uint32_t character_size, float width) {
    auto result = std::string("");
    auto word = std::string("");
    auto line_width = 0.f;
    auto word_width = 0.f;
    auto last = '\0';

    for (const char c : src) {
        const auto glyph = font.getGlyph(c, character_size, false);
        auto diff = glyph.advance;
        if (last != '\0') {
            diff += font.getKerning(last, c, character_size);
        }
        if (line_width + word_width + diff <= width * 1.125f) {
            word.push_back(c);
            word_width += diff;
            if (c == ' ' || c == '\t' || c == '\n' || c == '\r') {
                extend_str(result, word);
                word = "";
                line_width += word_width;
                word_width = 0.f;
            }
        } else if (word_width + diff <= width * 1.125f) {
            result.push_back('\n');
            line_width = 0.f;
            word.push_back(c);
            word_width += diff;
            if (c == ' ' || c == '\t' || c == '\n' || c == '\r') {
                extend_str(result, word);
                word = "";
                line_width += word_width;
                word_width = 0.f;
            }
        } else {
            extend_str(result, word);
            word = "";
            line_width = glyph.advance;
            word_width = 0.f;
            result.push_back('\n');
            result.push_back(c);
        }
        last = c;
    }

    if (line_width + word_width <= width * 1.125f) {
        extend_str(result, word);
    } else {
        result.push_back('\n');
        extend_str(result, word);
    }

    return result;
}

void TextWidget::apply_text_alignment() {
    m_label.setString(wrapped(m_label_value, get_style().font, get_character_size(), get_bounds().size.x - 2.f * m_text_padding));

    const auto c_size = get_size();
    const auto t_size = m_label.getLocalBounds().size;
    switch (m_text_alignment) {
    case Alignment::TopLeft:
        m_label.setPosition({m_text_padding, m_text_padding}); break;
    case Alignment::CenterLeft:
        m_label.setPosition({m_text_padding, c_size.y * 0.5f - t_size.y * 0.5f}); break;
    case Alignment::BottomLeft:
        m_label.setPosition({m_text_padding, c_size.y - t_size.y - m_text_padding}); break;
    case Alignment::TopCenter:
        m_label.setPosition({c_size.x * 0.5f - t_size.x * 0.5f, m_text_padding}); break;
    case Alignment::Center:
        m_label.setPosition(c_size * 0.5f - t_size * 0.5f); break;
    case Alignment::BottomCenter:
        m_label.setPosition({c_size.x * 0.5f - t_size.x * 0.5f, c_size.y - t_size.y - m_text_padding}); break;
    case Alignment::TopRight:
        m_label.setPosition({c_size.x - t_size.x - m_text_padding, m_text_padding}); break;
    case Alignment::CenterRight:
        m_label.setPosition({c_size.x - t_size.x - m_text_padding, c_size.y * 0.5f - t_size.y * 0.5f}); break;
    case Alignment::BottomRight:
        m_label.setPosition({c_size.x - t_size.x - m_text_padding, c_size.y - t_size.y - m_text_padding}); break;
    }
    m_label.move(sf::Vector2f(0.f, -5.f));
}

}

