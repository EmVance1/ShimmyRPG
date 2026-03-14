#pragma once
#include <SFML/Graphics.hpp>
#include <sfutil/sfutil.h>


namespace gui {

namespace lo {

struct XPos {
    enum class Alignment {
        Left, Center, Right
    };
    Alignment alignment;
    float offset;

    float get_relative(const sf::FloatRect& container, const sf::Vector2f& size) const;
    float get_absolute(const sf::FloatRect& container, const sf::Vector2f& size) const {
        return container.position.x + get_relative(container, size);
    }
};

struct YPos {
    enum class Alignment {
        Top, Center, Bottom
    };
    Alignment alignment;
    float offset;


    float get_relative(const sf::FloatRect& container, const sf::Vector2f& size) const;
    float get_absolute(const sf::FloatRect& container, const sf::Vector2f& size) const {
        return container.position.x + get_relative(container, size);
    }
};

XPos left   (float offset);
XPos xcenter(float offset);
XPos right  (float offset);

YPos top    (float offset);
YPos ycenter(float offset);
YPos bottom (float offset);

struct Size {
    enum class Mode {
        Absolute,
        Percent,
        Fit,
    };
    Mode mode;
    float amount;
};

Size absolute(float amount);
Size percent (float amount);
Size fitcontent();

}


struct Position {
    lo::XPos x_pos;
    lo::YPos y_pos;

    Position(const sf::Vector2f& offset) : x_pos(lo::left(offset.x)), y_pos(lo::top(offset.y)) {}
    Position(lo::XPos _xpos, lo::YPos _ypos) : x_pos(_xpos), y_pos(_ypos) {}

    Position shifted(const sf::Vector2f& off) const {
        return Position{
            { x_pos.alignment, x_pos.offset + off.x },
            { y_pos.alignment, y_pos.offset + off.y }
        };
    }

    sf::Vector2f get_relative(const sf::FloatRect& container, const sf::Vector2f& size) const;
    sf::Vector2f get_absolute(const sf::FloatRect& container, const sf::Vector2f& size) const {
        return container.position + get_relative(container, size);
    }
};

struct Sizing {
    lo::Size x_size;
    lo::Size y_size;

    Sizing(const sf::Vector2f& amount) : x_size(lo::absolute(amount.x)), y_size(lo::absolute(amount.y)) {}
    Sizing(lo::Size _xsize, lo::Size _ysize) : x_size(_xsize), y_size(_ysize) {}

    sf::Vector2f get_size(const sf::FloatRect& container) const;
};


namespace lo {

Position center(const sf::Vector2f& offset);
Sizing fill();

}

}

