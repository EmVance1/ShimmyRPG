#include "pch.h"
#include "gui/list.h"
#include "gui/button.h"


namespace gui {

ButtonList::ButtonList(const Position& position, const sf::Vector2f& size, const Style& style)
    : Container(position, sf::Vector2f(size.x, 0.f), style), m_blocksize(size)
{}

ButtonList::ButtonList(const Position& position, const sf::Vector2f& size, const Style& style, const std::vector<std::string>& values)
    : Container(position, sf::Vector2f(size.x, 0.f), style), m_blocksize(size)
{
    for (size_t i = 0; i < values.size(); i++) {
        auto b = Button::create(Position::topleft({0.f, get_size().y}), size, style, values[i]);
        b->set_container(this);
        add_widget(b);
        set_size(get_size() + sf::Vector2f(0.f, size.y));
    }
}


std::shared_ptr<Button> ButtonList::add_button(const std::string& value) {
    auto b = Button::create(Position::topleft({0.f, get_size().y}), m_blocksize, get_style(), value);
    b->set_container(this);
    add_widget(b);
    set_size(get_size() + sf::Vector2f(0.f, m_blocksize.y));
    return b;
}

std::shared_ptr<Button> ButtonList::add_button(const std::string& value, std::function<void()> callback) {
    auto b = Button::create(Position::topleft({0.f, get_size().y}), m_blocksize, get_style(), value);
    b->set_container(this);
    b->set_callback(callback);
    add_widget(b);
    set_size(get_size() + sf::Vector2f(0.f, m_blocksize.y));
    return b;
}

}
