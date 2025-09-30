#pragma once
#include <SFML/Graphics.hpp>
#include <memory>
#include "../style.h"


namespace gui {

class Widget : public sf::Drawable, public std::enable_shared_from_this<Widget> {
public:
    static sf::Vector2u VIEWPORT_SIZE;

private:
    sf::RectangleShape m_background;
    sf::Transform m_transform;
    const Widget* p_container = nullptr;
    bool m_enabled = true;
    bool m_visible = true;
    bool m_hovered = false;
    bool m_destroy = false;
    bool m_inherit_style = true;
    int32_t m_sortinglayer = 0;
    Position m_position;
    sf::FloatRect m_bounds;

    const Style* m_style;

protected:
    sf::Vector2f compute_absolute_position() {
        if (p_container) {
            return m_position.get_absolute_topleft(p_container->get_bounds(), m_bounds.size);
        } else {
            return m_position.get_absolute_topleft(sf::FloatRect({0, 0}, sf::Vector2f(VIEWPORT_SIZE)), m_bounds.size);
        }
    }
    sf::Vector2f compute_relative_position() {
        if (p_container) {
            return m_position.get_relative_topleft(p_container->get_bounds(), m_bounds.size);
        } else {
            return m_position.get_relative_topleft(sf::FloatRect({0, 0}, sf::Vector2f(VIEWPORT_SIZE)), m_bounds.size);
        }
    }
    void update_position() {
        m_bounds.position = compute_absolute_position();
        m_transform = sf::Transform().translate(compute_relative_position());
    }

public:
    Widget(const Position& position, const sf::Vector2f& size, const Style& style)
        : m_position(position), m_bounds(compute_absolute_position(), size), m_style(&style)
    {
        m_transform.translate(compute_relative_position());
        m_background.setSize(size);
        m_background.setFillColor(style.background_color_1);
        m_background.setOutlineColor(style.outline_color_1);
        m_background.setOutlineThickness(style.outline_width);
        if (style.textured) {
            m_background.setTexture(&style.background_texture);
            m_background.setTextureRect(style.default_cell);
        }
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

    void set_sorting_layer(int32_t layer) { m_sortinglayer = layer; }
    int32_t get_sorting_layer() const { return m_sortinglayer; }

    void set_style_inherited(bool inherit) { m_inherit_style = inherit; }
    bool is_style_inherited() const { return m_inherit_style; }

    void set_background_color(const sf::Color& color) { m_background.setFillColor(color); }
    void set_background_texture(const sf::Vector2i& cell) {
        m_background.setTextureRect(sf::IntRect{ cell, (sf::Vector2i)m_bounds.size });
    }
    void set_background_texture(const sf::IntRect& rect) {
        m_background.setTextureRect(rect);
    }

    void destroy() { m_destroy = true; }
    bool is_destroyed() const { return m_destroy; }

    bool is_hovered() const { return m_hovered; }

    const sf::FloatRect& get_bounds() const { return m_bounds; }
    const sf::Transform& get_transform() const { return m_transform; }

    virtual void set_style(const Style& style) { m_style = &style; }
    virtual const Style& get_style() const { return *m_style; }

    virtual void set_position(const sf::Vector2f& position) { set_position(Position::topleft(position)); }
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
        if (auto mmv = event.getIf<sf::Event::MouseMoved>()) {
            m_hovered = m_bounds.contains(sf::Vector2f(mmv->position));
        } else if (auto mbp = event.getIf<sf::Event::MouseButtonPressed>()) {
            m_hovered = m_bounds.contains(sf::Vector2f(mbp->position));
        } if (auto mbr = event.getIf<sf::Event::MouseMoved>()) {
            m_hovered = m_bounds.contains(sf::Vector2f(mbr->position));
        }
        return m_hovered;
    }

    virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const {
        states.transform *= get_transform();
        target.draw(m_background, states);
    }
};

}
