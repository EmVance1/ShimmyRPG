#pragma once
#include <SFML/Graphics.hpp>
#include <sfutil/sfutil.h>
#include "motion/tracker.h"
#include "navmesh/types.h"
#include "sfutil/animation.h"
#include "util/vechash.h"
#include "action.h"
#include "stats.h"
#include "item.h"


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
    std::string m_script_id;
    bool m_is_offstage = false;

    sfu::AlphaMap m_bitmap;
    sfu::AnimatedSprite m_sprite;
    sfu::AnimatedSprite m_outline_sprite;

    PathTracker m_tracker;
    SortBoundary m_boundary;
    sfu::FloatCircle m_collider;

    bool m_is_character = true;
    bool m_is_hovered = false;

    std::unordered_set<std::string> m_tags;
    std::vector<ContextAction> m_actions;
    std::string m_dialogue_file;

    std::unordered_map<sf::Vector2u, ItemEntry> m_inventory;
    EntityStats m_stats;
    uint32_t m_coin = 0;

private:
    static const sfu::TextureAtlas& default_texture() { static sfu::TextureAtlas tex(sf::Image({1, 1}, sf::Color::White), {1, 1}); return tex; }

public:
    Entity() : m_sprite(Entity::default_texture()), m_outline_sprite(Entity::default_texture()), m_tracker(nullptr) {}
    Entity(const std::string& id,
           const sfu::TextureAtlas& texture,
           const sfu::TextureAtlas& outline,
           const sfu::AlphaMap& bitmap,
           const nav::NavMesh* pathfinder,
           bool character);

    const std::string& get_id() const;
    const sfu::AnimatedSprite& get_sprite() const;
    const sfu::AnimatedSprite& get_outline_sprite() const;
    sf::FloatRect get_AABB() const;
    sfu::FloatCircle get_trigger_collider() const;

    void set_animation(size_t index);
    void set_position(const sf::Vector2f& position, const sf::Transform& cart_to_iso);
    void set_sprite_position(const sf::Vector2f& position);

    void set_sorting_boundary(const sf::Vector2f& pos);
    void set_sorting_boundary(const sf::Vector2f& left, const sf::Vector2f& right);
    SortBoundary get_sorting_boundary() const;

    void set_script_id(const std::string& id) { m_script_id = id; }
    const std::string& get_script_id() const { return m_script_id; }

    void set_dialogue(const std::string& filename);
    const std::string& get_dialogue() const;

    const PathTracker& get_tracker() const { return m_tracker; }
    PathTracker& get_tracker() { return m_tracker; }

    const std::vector<ContextAction>& get_actions() const { return m_actions; }
    std::vector<ContextAction>& get_actions() { return m_actions; }

    const std::unordered_set<std::string>& get_tags() const { return m_tags; }
    std::unordered_set<std::string>& get_tags() { return m_tags; }

    const EntityStats& get_stats() const { return m_stats; }
    EntityStats& get_stats() { return m_stats; }

    void set_offstage(bool offstage) { m_is_offstage = offstage; }
    bool is_offstage() const { return m_is_offstage; }

    bool is_character() const { return m_is_character; }

    bool contains(const sf::Vector2f& point) const;
    void set_hovered(bool hovered);
    bool is_hovered() const { return m_is_hovered; }

    void update(const sf::Transform& cart_to_iso);
};

