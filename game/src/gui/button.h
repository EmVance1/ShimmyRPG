#pragma once
#include <SFML/Graphics.hpp>
#include <functional>
#include "widget.h"


namespace gui {

class Button : public Widget {
private:
    sf::RectangleShape m_shape;
    sf::Text m_label;
    std::string m_value;
    bool m_clicked = false;
    bool m_has_callback = false;
    std::function<void()> m_callback;

public:
    Button(const Position& position, const sf::Vector2f& size, const Style& style, const std::string& label);

    static std::shared_ptr<Button> create(const Position& position, const sf::Vector2f& size, const Style& style, const std::string& label) {
        return std::make_shared<Button>(position, size, style, label);
    }

    void set_label(const std::string& label) { m_value = label; m_label.setString(label); }
    const std::string& get_label() const { return m_value; }

    bool get_value() const { return m_clicked; }

    void set_callback(std::function<void()> func) { m_callback = func; m_has_callback = true; }

    void update() override;
    bool handle_event(const sf::Event& event) override;
    void draw(sf::RenderTarget& target, sf::RenderStates states) const override;
};

}
