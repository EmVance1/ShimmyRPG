#pragma once
#include <SFML/Graphics.hpp>
#include "widget.h"
#include "button.h"


namespace gui {

class ButtonList : public Widget {
private:
    std::vector<Button> m_options;
    size_t m_selected = 0;
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

    void add_button(const std::string& value);
    void add_button(const std::string& value, std::function<void()> callback);

    size_t get_index() const { return m_selected; }

    void set_style(const Style& style) override;
    void set_position(const Position& position) override;

    void update() override;
    bool handle_event(const sf::Event& event) override;
    void draw(sf::RenderTarget& target, sf::RenderStates states) const override;
};

}
