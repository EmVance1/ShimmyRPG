#pragma once
#include <SFML/Graphics.hpp>
#include <functional>
#include "bases/textwidget.h"


namespace gui {

class Button : public TextWidget {
private:
    bool m_has_callback = false;
    std::function<void()> m_callback;

public:
    Button(const Position& position, const sf::Vector2f& size, const Style& style, const std::string& label);

    static std::shared_ptr<Button> create(const Position& position, const sf::Vector2f& size, const Style& style, const std::string& label) {
        return std::make_shared<Button>(position, size, style, label);
    }

    void set_callback(std::function<void()> func) { m_callback = func; m_has_callback = true; }

    bool handle_event(const sf::Event& event) override;
};

}
