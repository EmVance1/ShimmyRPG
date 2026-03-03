#pragma once
#include <SFML/Graphics.hpp>
#include <sfutil/sfutil.h>
#include <navmesh/lib.h>
#include <unordered_set>
#include "data/bundler.h"
#include "objects/inventory.h"
#include "game/simulate.h"
#include "stats.h"


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
    }
};


class Entity {
public:
    inline static constexpr uint32_t INVALID = UINT32_MAX;
    enum class Action {
        SetPath,
        MoveTo,
        Speak,
        Examine,
        UsePortal,
    };
    enum class BusyTarget {
        None = 0,
        DestReached,
        SceneChange,
    };

private:
    std::string m_id;
    std::string m_name;
    std::unordered_set<std::string> m_tags;

    bool m_is_playable = false;
    bool m_is_offstage = false;
    bool m_is_hovered = false;
    bool m_is_ghost = false;
    BusyTarget m_busy_target = BusyTarget::None;

    sfu::AlphaMap m_bitmap;
    sfu::AnimatedSprite m_sprite;
    sfu::AnimatedSprite m_outline_sprite;

    nav::Agent m_tracker;
    SortBoundary m_boundary;
    sfu::FloatCircle m_collider;

    std::vector<Action> m_actions;
    std::deque<shmy::sim::Event> m_queue;

    EntityStats m_stats;
    Inventory m_inventory;
    std::string m_dialogue;
    std::string m_examination;

    inline static const sfu::TextureAtlas default_texture{ sf::Image({1, 1}, sf::Color::White), {1, 1} };

public:
    Entity() : m_sprite(shmy::data::Bundler::nullsheet()), m_outline_sprite(shmy::data::Bundler::nullsheet()), m_tracker(nullptr) {}
    Entity(const std::string& id,
           const std::string& name,
           const sfu::TextureAtlas& texture,
           const sfu::TextureAtlas& outline,
           const sfu::AlphaMap& bitmap,
           const nav::Mesh* pathfinder);

    const std::string& id() const;
    const std::string& name() const;
    const sfu::AnimatedSprite& get_sprite() const;
    const sfu::AnimatedSprite& get_outline_sprite() const;
    sf::FloatRect get_AABB() const;
    sfu::FloatCircle get_trigger_collider() const;

    void set_animation(size_t index);
    void set_position(const sf::Vector2f& position, const sf::Transform& world_to_screen);
    nav::Vector2f get_world_position(const sf::Transform& screen_to_world) const;
    sf::Vector2f get_tooltip_position() const;
    void set_sprite_position(const sf::Vector2f& position);
    void set_navmesh(const nav::Mesh* pathfinder);

    void set_sorting_boundary(const sf::Vector2f& pos);
    void set_sorting_boundary(const sf::Vector2f& left, const sf::Vector2f& right);
    SortBoundary get_sorting_boundary() const;

    const std::string& get_dialogue() const { return m_dialogue; }
    std::string& get_dialogue() { return m_dialogue; }

    const std::string& get_examination() const { return m_examination; }
    std::string& get_examination() { return m_examination; }

    const nav::Agent& get_tracker() const { return m_tracker; }
    nav::Agent& get_tracker() { return m_tracker; }

    const std::unordered_set<std::string>& get_tags() const { return m_tags; }
    std::unordered_set<std::string>& get_tags() { return m_tags; }

    const std::vector<Action>& get_actions() const { return m_actions; }
    std::vector<Action>& get_actions() { return m_actions; }

    void push_action(const shmy::sim::Event& action);
    void push_action_to_front(const shmy::sim::Event& action);
    std::optional<shmy::sim::Event> poll_action();

    const EntityStats& get_stats() const { return m_stats; }
    EntityStats& get_stats() { return m_stats; }

    const Inventory& get_inventory() const { return m_inventory; }
    Inventory& get_inventory() { return m_inventory; }

    void set_playable(bool playable) { m_is_playable = playable; }
    bool is_playable() const { return m_is_playable; }

    void set_offstage(bool offstage) { m_is_offstage = offstage; }
    bool is_offstage() const { return m_is_offstage; }

    void set_ghost(bool ghost) { m_is_ghost = ghost; }
    bool is_ghost() const { return m_is_ghost; }

    void set_busy_until(BusyTarget busy) { m_busy_target = busy; }
    void busy_reached(BusyTarget busy) { if (m_busy_target == busy) m_busy_target = BusyTarget::None; }

    bool is_interactible() const { return !(m_is_offstage || m_is_ghost); }
    bool is_character() const { return m_tracker.is_available(); }

    bool contains(const sf::Vector2f& point) const;
    void set_hovered(bool hovered);
    bool is_hovered() const { return m_is_hovered; }

    void update(const sf::Transform& world_to_screen);
};

