#pragma once
#include <SFML/Graphics.hpp>
#include <unordered_map>
#include <memory>
#include "../style.h"
#include "widget.h"


namespace gui {

class Container : public Widget {
private:
    std::unordered_map<std::string, std::shared_ptr<Widget>> m_children;

public:
    Container(const Position& position, const sf::Vector2f& size, const Style& style);

    static std::shared_ptr<Container> create(const Position& position, const sf::Vector2f& size, const Style& style) {
        return std::make_shared<Container>(position, size, style);
    }

    void add_widget(const std::string& name, const std::shared_ptr<Widget>& widget, bool inherit_style = true);
    void add_widget(const std::shared_ptr<Widget>& widget, bool inherit_style = true);
    std::shared_ptr<Widget> get_widget(const std::string& id);
    std::shared_ptr<const Widget> get_widget(const std::string& id) const;
    template<typename T>
    std::shared_ptr<T> get_widget(const std::string& id) {
        return std::dynamic_pointer_cast<T>(get_widget(id));
    }
    template<typename T>
    std::shared_ptr<const T> get_widget(const std::string& id) const {
        return std::dynamic_pointer_cast<const T>(get_widget(id));
    }
    bool has_widget(const std::string& id) const;
    std::shared_ptr<Widget> remove_widget(const std::string& id);

    virtual void clear() { m_children.clear(); }

    void set_position(const Position& position) override;
    void set_style(const Style& style) override;

    void update() override;
    bool handle_event(const sf::Event& event) override;
    void draw(sf::RenderTarget& target, sf::RenderStates states) const override;
};

}
