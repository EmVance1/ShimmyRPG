#pragma once
#include <SFML/Graphics.hpp>
#include "widget.h"


namespace gui {

class Image : public Widget {
private:
    sf::RectangleShape m_shape;

public:
    Image(const Position& position, const sf::Vector2f& size, const Style& style, const sf::Texture& texture);

    static std::shared_ptr<Image> create(const Position& position, const sf::Vector2f& size, const Style& style, const sf::Texture& texture) {
        return std::make_shared<Image>(position, size, style, texture);
    }

    void update() override {}
    bool handle_event(const sf::Event&) override { return false; }
    void draw(sf::RenderTarget& target, sf::RenderStates states) const override;
};

}
