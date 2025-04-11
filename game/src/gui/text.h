#pragma once
#include <SFML/Graphics.hpp>
#include "widget.h"


namespace gui {

class Text : public Widget {
private:
    sf::RectangleShape m_shape;
    sf::Text m_label;
    std::string m_value;
    bool m_background_enabled = false;

public:
    Text(const Position& position, uint32_t character_size, const Style& style, const std::string& label);

    static std::shared_ptr<Text> create(const Position& position, uint32_t character_size, const Style& style, const std::string& label) {
        return std::make_shared<Text>(position, character_size, style, label);
    }

    void set_label(const std::string& label) { m_value = label; m_label.setString(label); }
    const std::string& get_label() const { return m_value; }

    void set_background_enabled(bool enabled) { m_background_enabled = enabled; }
    bool is_background_enabled() const { return m_background_enabled; }

    void update() override {}
    bool handle_event(const sf::Event&) override { return false; }
    void draw(sf::RenderTarget& target, sf::RenderStates states) const override;
};

}
