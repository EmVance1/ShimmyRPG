#include "pch.h"
#include <utf8.h>
#include "gui/bases/textwidget.h"


namespace gui {

static std::vector<std::u32string> wrapped(const std::u32string& src, const sf::Font& font, uint32_t character_size, float width) {
    if (src.empty()) return {};
    auto lines = std::vector<std::u32string>();
    lines.emplace_back();
    auto word = std::u32string();
    auto line_width = 0.f;
    auto word_width = 0.f;
    auto last = U'\0';

    for (const char32_t c : src) {
        auto result = &lines.back();
        const auto glyph = font.getGlyph(c, character_size, false);
        const auto diff = glyph.advance + font.getKerning(last, c, character_size);
        if (line_width + word_width + diff <= width) {
            word.push_back(c);
            word_width += diff;
            if (c == ' ' || c == '\t' || c == '\n' || c == '\r') {
                result->insert(result->end(), word.begin(), word.end());
                word.clear();
                line_width += word_width;
                word_width = 0.f;
            }
        } else if (word_width + diff <= width) {
            result = &lines.emplace_back();
            line_width = 0.f;
            word.push_back(c);
            word_width += diff;
            if (c == U' ' || c == U'\t' || c == U'\n' || c == U'\r') {
                result->insert(result->end(), word.begin(), word.end());
                word.clear();
                line_width += word_width;
                word_width = 0.f;
            }
        } else {
            result->insert(result->end(), word.begin(), word.end());
            word.clear();
            line_width = glyph.advance;
            word_width = 0.f;
            result->push_back(U'\n');
            result->push_back(c);
        }
        last = c;
    }

    auto result = &lines.back();
    if (line_width + word_width <= width) { // * 1.125f) {
        result->insert(result->end(), word.begin(), word.end());
    } else {
        result->push_back(U'\n');
        result->insert(result->end(), word.begin(), word.end());
    }

    return lines;
}

void TextWidget::update_textalign() {
    m_labels.clear();
    const auto lines = wrapped(m_label_value, get_style().font, m_character_size, get_bounds().size.x - 2.f * m_text_padding);
    for (size_t i = 0; i < lines.size(); i++) {
        auto& label = m_labels.emplace_back(get_style().font, lines[i], m_character_size);
        label.setPosition({ 0.f, std::floor((float)i * ((float)m_character_size + m_line_spacing) - 5.f) });
        label.setFillColor(get_style_variant().text);
    }
}

void TextWidget::update_textpos() {
    for (auto& l : m_labels) {
        const auto t_size = sf::Vector2f(l.getGlobalBounds().size.x, (float)m_character_size);
        const auto t_pos = m_textpos.get_absolute(get_bounds(), t_size);
        if (m_textpos.x_pos.alignment == lo::XPos::Alignment::Center) {
            const auto p = l.getPosition() + t_pos + sf::Vector2f(0, m_text_padding);
            l.setPosition({ std::floor(p.x), std::floor(p.y) });
        } else {
            const auto p = l.getPosition() + t_pos + sf::Vector2f(m_text_padding, m_text_padding);
            l.setPosition({ std::floor(p.x), std::floor(p.y) });
        }
    }
}

void TextWidget::update_transform() {
    const auto parent_box = get_container() ? get_container()->get_bounds() : sf::FloatRect({ 0.f, 0.f }, viewport);
    const auto x_size = get_sizing().x_size;
    switch (x_size.mode) {
    case lo::Size::Mode::Absolute:
        m_bounds.size.x = x_size.amount;
        break;
    case lo::Size::Mode::Percent:
        m_bounds.size.x = x_size.amount * parent_box.size.x * 0.01f;
        break;
    case lo::Size::Mode::Fit:
        m_bounds.size.x = x_size.amount;
        break;
    }
    update_textalign();
    const auto y_size = get_sizing().y_size;
    switch (y_size.mode) {
    case lo::Size::Mode::Absolute:
        m_bounds.size.y = y_size.amount;
        break;
    case lo::Size::Mode::Percent:
        m_bounds.size.y = y_size.amount * parent_box.size.y * 0.01f;
        break;
    case lo::Size::Mode::Fit: {
        const auto line_space = m_labels.empty() ? 0.f : (float)(m_labels.size() - 1) * m_line_spacing;
        m_bounds.size.y = (float)m_labels.size() * (float)m_character_size + 2.f * m_text_padding + line_space;
        break; }
    }
    m_bounds.position = get_position().get_absolute(parent_box, m_bounds.size);
    update_textpos();
    m_background.setSize(m_bounds.size);
    m_background.setPosition(m_bounds.position);
}


TextWidget::TextWidget(const Position& position, const Sizing& sizing, const Style& style)
    : Widget(position, sizing, style)
{
    update_transform();
}


void TextWidget::set_style(const Style& style) {
    Widget::set_style(style);
    for (auto& l : m_labels) {
        l.setFillColor(get_style_variant().text);
    }
}

void TextWidget::set_style_variant(size_t variant) {
    Widget::set_style_variant(variant);
    for (auto& l : m_labels) {
        l.setFillColor(get_style_variant().text);
    }
}


void TextWidget::set_character_size(uint32_t character_size) {
    m_character_size = character_size;
    update_transform();
}

void TextWidget::set_text_position(Position position) {
    m_textpos = position;
    update_transform();
}

void TextWidget::set_text_padding(float padding) {
    m_text_padding = padding;
    update_transform();
}

void TextWidget::set_text_line_spacing(float spacing) {
    m_line_spacing = spacing;
    update_transform();
}

void TextWidget::set_text_char_spacing(float spacing) {
    m_char_spacing = spacing;
    update_transform();
}

void TextWidget::set_label(const std::string& label) {
    m_label_value = utf8::utf8to32(label);
    update_transform();
}

void TextWidget::set_label(const std::u32string& label) {
    m_label_value = label;
    update_transform();
}

void TextWidget::draw(sf::RenderTarget& target, sf::RenderStates states) const {
    Widget::draw(target, states);
    for (const auto& l : m_labels) {
        target.draw(l, states);
    }
}

}

