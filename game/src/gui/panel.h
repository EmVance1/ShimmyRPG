#pragma once
#include <SFML/Graphics.hpp>
#include <unordered_map>
#include <memory>
#include "style.h"
#include "widget.h"


namespace gui {

class Panel : public Widget {
private:
    sf::RectangleShape m_background;
    std::unordered_map<std::string, std::shared_ptr<Widget>> m_widgets;

public:
    void add_widget(const std::string& name, const std::shared_ptr<Widget>& widget, bool inherit_style = true);
    std::shared_ptr<Widget> get_widget(const std::string& id);
    std::shared_ptr<const Widget> get_widget(const std::string& id) const;
    bool has_widget(const std::string& id) const;
    std::shared_ptr<Widget> remove_widget(const std::string& id);

    void set_position(const Position& position) override;
    void set_style(const Style& style) override;

    void update() override;
    bool handle_event(const sf::Event& event) override;
    void draw(sf::RenderTarget& target, sf::RenderStates states) const override;
};

}
