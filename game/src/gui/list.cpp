#include "pch.h"
#include "list.h"


namespace gui {

ButtonList::ButtonList(const Position& position, const sf::Vector2f& size, const Style& style)
    : Widget(position, sf::Vector2f(size.x, 0.f), style), m_blocksize(size)
{}

ButtonList::ButtonList(const Position& position, const sf::Vector2f& size, const Style& style, const std::vector<std::string>& values)
    : Widget(position, sf::Vector2f(size.x, 0.f), style), m_blocksize(size)
{
    for (const auto& value : values) {
        auto& b = m_options.emplace_back(
            Position::topleft({0.f, m_bounds.size.y}),
            m_blocksize,
            *m_style,
            value
        );
        b.set_container(this);
        m_bounds.size.y += m_blocksize.y;
    }
}


void ButtonList::add_button(const std::string& value) {
    auto& b = m_options.emplace_back(
        Position::topleft({0.f, (float)m_options.size() * 30.f}),
        m_blocksize,
        *m_style,
        value
    );
    b.set_container(this);
    m_bounds.size.y += m_blocksize.y;
}

void ButtonList::add_button(const std::string& value, std::function<void()> callback) {
    auto& b = m_options.emplace_back(
        Position::topleft({0.f, (float)m_options.size() * 30.f}),
        m_blocksize,
        *m_style,
        value
    );
    b.set_container(this);
    b.set_callback(callback);
    m_bounds.size.y += m_blocksize.y;
}


void ButtonList::set_style(const Style& style) {
    Widget::set_style(style);
    for (auto& op : m_options) {
        op.set_style(style);
    }
}

void ButtonList::set_position(const Position& position) {
    Widget::set_position(position);
    float h = 0.f;
    for (auto& op : m_options) {
        op.set_position(Position::topleft({0.f, h}));
        h += m_blocksize.y;
    }
}


void ButtonList::update() {
    for (auto& op : m_options) {
        op.set_container(this);
        op.update();
    }
}

bool ButtonList::handle_event(const sf::Event& event) {
    Widget::handle_event(event);
    for (size_t i = 0; i < m_options.size(); i++) {
        if (m_options[i].handle_event(event)) {
            m_selected = i;
            return true;
        }
    }

    return false;
}

void ButtonList::draw(sf::RenderTarget& target, sf::RenderStates states) const {
    states.transform *= get_transform();
    for (const auto& op : m_options) {
        target.draw(op, states);
    }
}

}
