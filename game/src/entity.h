#pragma once
#include <SFML/Graphics.hpp>
#include <sfutil/sfutil.h>
#include "motion/tracker.h"


struct SortBoundary {
    sf::Vector2f left;
    sf::Vector2f right;
    bool is_point = true;

    float get_threshold(const sf::Vector2f& point) const;
    auto operator<=>(const SortBoundary& b) const {
        if (is_point && b.is_point) {
            return left.y <=> b.left.y;
        } else if (is_point) {
            return left.y <=> b.get_threshold(left);
        } else if (b.is_point) {
            return get_threshold(b.left) <=> b.left.y;
        } else {
            return std::min(left.y, left.y) <=> std::min(b.left.y, b.right.y);
        }
        return std::partial_ordering::equivalent;
    }
};


class Entity {
private:
    sfu::AlphaMap m_bitmap;
    const sf::Texture* p_texture = nullptr;
    const sf::Texture* p_outline = nullptr;
    std::string m_id;
    sf::Sprite m_sprite;
    PathTracker m_tracker;
    SortBoundary m_boundary;
    sfu::FloatCircle m_collider;
    bool m_is_character = true;
    bool m_is_hovered = false;

public:
    static const sf::Texture& default_texture() { static sf::Texture tex(sf::Image({1, 1}, sf::Color::White)); return tex; }

public:
    Entity() : m_sprite(Entity::default_texture()), m_tracker(nullptr, 1.f) {}
    Entity(const sfu::AlphaMap& bitmap,
           const sf::Texture& texture,
           const sf::Texture& outline,
           const std::string& id,
           const SpatialGraph2d* pathfinder,
           float pathscale, bool character);

    const std::string& get_id() const;
    const sf::Sprite& get_sprite() const;
    SortBoundary get_boundary() const;
    sf::FloatRect get_AABB() const;
    sfu::FloatCircle get_trigger_collider() const;

    void set_sprite_position(const sf::Vector2f& position);
    void set_position(const sf::Vector2f& position, const sf::Transform& cart_to_iso);

    void set_sorting_boundary(const sf::Vector2f& pos);
    void set_sorting_boundary(const sf::Vector2f& left, const sf::Vector2f& right);

    bool set_tracker_target(const sf::Vector2f& cartesian);
    sf::Vector2f get_tracker_target() const;
    void set_tracker_speed(float speed);
    bool is_moving() const;
    const PathTracker& get_tracker() const { return m_tracker; }

    bool is_character() const { return m_is_character; }

    bool contains(const sf::Vector2f& point) const;
    void set_hovered(bool hovered);
    bool is_hovered() const { return m_is_hovered; }

    void update_motion(const sf::Transform& cart_to_iso);
};

