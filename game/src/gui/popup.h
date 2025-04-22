#pragma once
#include <SFML/Graphics.hpp>
#include <memory>
#include "style.h"
#include "bases/container.h"


namespace gui {

class Popup : public Container {
public:
    Popup(const Position& position, const sf::Vector2f& size, const Style& style, const std::string& label);

    static std::shared_ptr<Popup> create(const Position& position, const sf::Vector2f& size, const Style& style, const std::string& label) {
        return std::make_shared<Popup>(position, size, style, label);
    }
};

}
