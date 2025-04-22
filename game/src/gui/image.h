#pragma once
#include <SFML/Graphics.hpp>
#include "bases/widget.h"


namespace gui {

class Image : public Widget {
public:
    Image(const Position& position, const sf::Vector2f& size, const Style& style, const sf::Texture& texture);

    static std::shared_ptr<Image> create(const Position& position, const sf::Vector2f& size, const Style& style, const sf::Texture& texture) {
        return std::make_shared<Image>(position, size, style, texture);
    }
};

}
