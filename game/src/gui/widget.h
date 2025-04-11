#pragma once
#include <SFML/Graphics.hpp>
#include "style.h"


namespace gui {

class Widget : public sf::Drawable {
private:
    sf::Transform m_transform;
    bool m_enabled = true;
    bool m_visible = true;
    bool m_hovered = false;
    bool m_inherit_style = true;
    int32_t m_sortinglayer = 0;
    const Widget* p_container = nullptr;

protected:
    Position m_position;
    sf::FloatRect m_bounds;
    const Style* m_style;

    sf::Vector2f compute_absolute_position() {
        if (p_container) {
            return m_position.get_absolute_topleft(p_container->get_bounds(), m_bounds.size);
        } else {
            return m_position.offset;
        }
    }
    sf::Vector2f compute_relative_position() {
        if (p_container) {
            return m_position.get_relative_topleft(p_container->get_bounds(), m_bounds.size);
        } else {
            return m_position.offset;
        }
    }
    void update_position() {
        m_bounds.position = compute_absolute_position();
        m_transform = sf::Transform().translate(compute_relative_position());
    }

public:
    Widget() = default;
    Widget(const Position& position, const sf::Vector2f& size, const Style& style)
        : m_position(position), m_bounds(compute_absolute_position(), size), m_style(&style)
    {
        m_transform.translate(compute_relative_position());
    }

    void set_container(const Widget* container) {
        p_container = container;
        update_position();
    }
    const Widget& get_container() { return *p_container; }

    void set_enabled(bool enabled) { m_enabled = enabled; }
    bool is_enabled() const { return m_enabled; }

    void set_visible(bool visible) { m_visible = visible; }
    bool is_visible() const { return m_visible; }

    bool is_hovered() const { return m_hovered; }

    void set_sorting_layer(int32_t layer) { m_sortinglayer = layer; }
    int32_t get_sorting_layer() const { return m_sortinglayer; }

    void set_style_inherited(bool inherit) { m_inherit_style = inherit; }
    bool is_style_inherited() const { return m_inherit_style; }

    virtual void set_style(const Style& style) { m_style = &style; }
    virtual const Style& get_style() const { return *m_style; }

    const sf::FloatRect& get_bounds() const { return m_bounds; }
    const sf::Transform& get_transform() const { return m_transform; }

    virtual void set_position(const sf::Vector2f& position) {
        set_position(Position::topleft(position));
    }
    virtual void set_position(const Position& position) {
        m_position = position;
        update_position();
    }
    virtual const sf::Vector2f& get_absolute_position() const { return m_bounds.position; }
    virtual const Position& get_position() const { return m_position; }

    virtual void set_size(const sf::Vector2f& size) { m_bounds.size = size; }
    virtual const sf::Vector2f& get_size() const { return m_bounds.size; }

    virtual void update() {}
    virtual bool handle_event(const sf::Event& event) {
        if (auto mv = event.getIf<sf::Event::MouseMoved>()) {
            m_hovered = m_bounds.contains(sf::Vector2f(mv->position));
        }
        return false;
    }
};

}
