#pragma once
#include <SFML/Graphics.hpp>
#include <sfutil/sfutil.h>
#include "motion/tracker.h"
#include "sfutil/animation.h"
#include "action.h"


struct SortBoundary {
    sf::Vector2f left;
    sf::Vector2f right;
    bool is_point = true;

    float get_threshold(const sf::Vector2f& point) const;
    sf::Vector2f get_center_of_mass() const;
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
    std::string m_id;
    const sfu::TextureAtlas* p_texture = nullptr;
    const sfu::TextureAtlas* p_outline = nullptr;
    sfu::AlphaMap m_bitmap;
    sfu::AnimatedSprite m_sprite;
    PathTracker m_tracker;
    SortBoundary m_boundary;
    sfu::FloatCircle m_collider;
    bool m_is_character = true;
    bool m_is_hovered = false;
    std::vector<ContextAction> m_actions;
    std::string m_dialogue_file;

public:
    static const sfu::TextureAtlas& default_texture() { static sfu::TextureAtlas tex(sf::Image({1, 1}, sf::Color::White), {1, 1}); return tex; }

public:
    Entity() : m_sprite(Entity::default_texture()), m_tracker(nullptr, 1.f) {}
    Entity(const std::string& id,
           const sfu::TextureAtlas& texture,
           const sfu::TextureAtlas& outline,
           const sfu::AlphaMap& bitmap,
           const SpatialGraph2d* pathfinder,
           float pathscale, bool character);

    const std::string& get_id() const;
    const sfu::AnimatedSprite& get_sprite() const;
    SortBoundary get_boundary() const;
    sf::FloatRect get_AABB() const;
    sfu::FloatCircle get_trigger_collider() const;

    void set_sprite_position(const sf::Vector2f& position);
    void set_animation(size_t index);
    void set_position(const sf::Vector2f& position, const sf::Transform& cart_to_iso);
    // void get_position(const sf::Transform& cart_to_iso);

    void set_sorting_boundary(const sf::Vector2f& pos);
    void set_sorting_boundary(const sf::Vector2f& left, const sf::Vector2f& right);

    void set_dialogue(const std::string& filename);
    const std::string& get_dialogue() const;

    const PathTracker& get_tracker() const { return m_tracker; }
    PathTracker& get_tracker() { return m_tracker; }

    const std::vector<ContextAction>& get_actions() const { return m_actions; }
    std::vector<ContextAction>& get_actions() { return m_actions; }

    bool is_character() const { return m_is_character; }

    bool contains(const sf::Vector2f& point) const;
    void set_hovered(bool hovered);
    bool is_hovered() const { return m_is_hovered; }

    void update_motion(const sf::Transform& cart_to_iso);
};

