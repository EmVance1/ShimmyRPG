#pragma once
#include <SFML/Graphics.hpp>
#include <functional>
#include "bases/container.h"
#include "button.h"


namespace gui {

class ButtonList : public Container {
private:
    sf::Vector2f m_blocksize;

public:
    ButtonList(const Position& position, const sf::Vector2f& size, const Style& style);
    ButtonList(const Position& position, const sf::Vector2f& size, const Style& style, const std::vector<std::string>& values);

    static std::shared_ptr<ButtonList> create(const Position& position, const sf::Vector2f& size, const Style& style) {
        return std::make_shared<ButtonList>(position, size, style);
    }
    static std::shared_ptr<ButtonList> create(const Position& position, const sf::Vector2f& size, const Style& style, const std::vector<std::string>& values) {
        return std::make_shared<ButtonList>(position, size, style, values);
    }

    std::shared_ptr<Button> add_button(const std::string& value);
    std::shared_ptr<Button> add_button(const std::string& value, std::function<void()> callback);
    void clear() { Container::clear(); set_size({get_size().x, 0}); }
};

}
