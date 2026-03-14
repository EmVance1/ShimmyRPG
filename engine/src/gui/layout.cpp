#include "pch.h"
#include "gui/layout.h"


namespace gui {

namespace lo {

float XPos::get_relative(const sf::FloatRect& container, const sf::Vector2f& size) const {
    switch (alignment) {
    case Alignment::Left:
        return offset;
    case Alignment::Center:
        return offset + container.size.x * 0.5f - size.x * 0.5f;
    case Alignment::Right:
        return offset + container.size.x - size.x;
    }
    return offset;
}

float YPos::get_relative(const sf::FloatRect& container, const sf::Vector2f& size) const {
    switch (alignment) {
    case Alignment::Top:
        return offset;
    case Alignment::Center:
        return offset + container.size.y * 0.5f - size.y * 0.5f;
    case Alignment::Bottom:
        return offset + container.size.y - size.y;
    }
    return offset;
}

XPos left   (float offset) { return XPos{ XPos::Alignment::Left,   offset }; }
XPos xcenter(float offset) { return XPos{ XPos::Alignment::Center, offset }; }
XPos right  (float offset) { return XPos{ XPos::Alignment::Right,  offset }; }

YPos top    (float offset) { return YPos{ YPos::Alignment::Top,    offset }; }
YPos ycenter(float offset) { return YPos{ YPos::Alignment::Center, offset }; }
YPos bottom (float offset) { return YPos{ YPos::Alignment::Bottom, offset }; }


Size absolute(float amount) { return Size{ Size::Mode::Absolute, amount }; }
Size percent (float amount) { return Size{ Size::Mode::Percent,  amount }; }
Size fitcontent() { return Size{ Size::Mode::Fit, {} }; }

}


sf::Vector2f Position::get_relative(const sf::FloatRect& container, const sf::Vector2f& size) const {
    return { x_pos.get_relative(container, size), y_pos.get_relative(container, size) };
}


sf::Vector2f Sizing::get_size(const sf::FloatRect& container) const {
    auto res = sf::Vector2f();
    switch (x_size.mode) {
    case lo::Size::Mode::Absolute:
        res.x = x_size.amount;
        break;
    case lo::Size::Mode::Percent:
        res.x = x_size.amount * container.size.x * 0.01f;
        break;
    case lo::Size::Mode::Fit:
        res.x = x_size.amount;
        break;
    }
    switch (y_size.mode) {
    case lo::Size::Mode::Absolute:
        res.y = y_size.amount;
        break;
    case lo::Size::Mode::Percent:
        res.y = y_size.amount * container.size.y * 0.01f;
        break;
    case lo::Size::Mode::Fit:
        res.y = y_size.amount;
        break;
    }
    return res;
}


namespace lo {

Position center(const sf::Vector2f& offset) {
    return { gui::lo::xcenter(offset.x), gui::lo::ycenter(offset.y) };
}
Sizing fill() {
    return { gui::lo::percent(100), gui::lo::percent(100) };
}

}


}
