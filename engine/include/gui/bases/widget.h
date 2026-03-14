#pragma once
#include <SFML/Graphics.hpp>
#include <memory>
#include "../style.h"
#include "../layout.h"


namespace gui {

class Widget : public sf::Drawable, public std::enable_shared_from_this<Widget> {
public:
    inline static sf::Vector2f viewport;

private:
    const Widget* p_container = nullptr;
    const Style* p_style = nullptr;
    size_t m_style_variant = 0;
    Position m_position;
    Sizing m_sizing;
    bool m_enabled = true;
    bool m_visible = true;
    bool m_hovered = false;
    bool m_destroy = false;
    int m_layer = 0;

protected:
    sfu::BorderShape m_background;
    sf::FloatRect m_bounds;

    virtual void update_transform() {
        const auto parent_box = p_container ? p_container->m_bounds : sf::FloatRect({ 0.f, 0.f }, viewport);
        m_bounds.size = m_sizing.get_size(parent_box);
        m_bounds.position = m_position.get_absolute(parent_box, m_bounds.size);
        m_background.setSize(m_bounds.size);
        m_background.setPosition(m_bounds.position);
    }

public:
    Widget(const Position& position, const Sizing& sizing, const Style& style)
        : p_style(&style), m_position(position), m_sizing(sizing)
    {
        update_transform();
        m_background.setFillColor(style.variant[0].bg_1);
        if (style.textured) {
            m_background.setTexture(&style.background_texture, {0, 0});
            m_background.setTextureRect({ {0, 0}, {1, 1} });
        }
    }

    void set_container(const Widget* container) {
        p_container = container;
        p_style = container->p_style;
        update_transform();
    }
    const Widget* get_container() { return p_container; }

    void set_enabled(bool enabled) { m_enabled = enabled; }
    bool is_enabled() const { return m_enabled; }

    void set_visible(bool visible) { m_visible = visible; }
    bool is_visible() const { return m_visible; }

    void set_sorting_layer(int32_t layer) { m_layer = layer; }
    int32_t get_sorting_layer() const { return m_layer; }

    void set_background_color(const sf::Color& color) { m_background.setColor(color); }
    const sf::Color& get_background_color() { return m_background.getFillColor(); }

    void set_background_texture(const sf::Vector2i& cell) {
        m_background.setTextureRect(sf::IntRect{ cell, (sf::Vector2i)m_bounds.size });
    }
    void set_background_texture(const sf::IntRect& rect) {
        m_background.setTextureRect(rect);
    }
    void set_border(const sf::Vector2f& dims) {
        m_background.setBorders(dims);
    }

    void destroy() { m_destroy = true; }
    bool is_destroyed() const { return m_destroy; }

    bool is_hovered() const { return m_hovered; }

    virtual void set_position(const Position& position) { m_position = position; update_transform(); }
    virtual const sf::Vector2f& get_absolute_position() const { return m_bounds.position; }
    virtual const Position& get_position() const { return m_position; }

    virtual void set_sizing(const Sizing& sizing) { m_sizing = sizing; update_transform(); }
    virtual const sf::Vector2f& get_absolute_size() const { return m_bounds.size; }
    virtual const Sizing& get_sizing() const { return m_sizing; }

    const sf::FloatRect& get_bounds() const { return m_bounds; }

    virtual void set_style(const Style& style) {
        p_style = &style;
        m_background.setFillColor(get_style_variant().bg_1);
    }
    const Style& get_style() const { return *p_style; }

    virtual void set_style_variant(size_t variant) {
        m_style_variant = variant;
        m_background.setFillColor(get_style_variant().bg_1);
    }
    const Style::Element& get_style_variant() const { return p_style->variant[m_style_variant]; }

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
        target.draw(m_background, states);
    }

    friend class Container;
};

}
