#pragma once
#include <SFML/Graphics.hpp>
#include <sfutil/sfutil.h>
#include <navmesh/lib.h>
#include <unordered_set>
#include "objects/inventory.h"
#include "game/simulate.h"
#include "stats.h"


namespace shmy {

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


class EntityList;

class Entity {
public:
    inline static constexpr uint32_t INVALID = UINT32_MAX;
    enum class Action {
        MoveTo,
        Speak,
        Examine,
        ExitScene,
    };
    enum Flag {
        Playable = 1,
        Offstage = 1 << 1,
        Hovered  = 1 << 2,
        Ghost    = 1 << 3,
    };

private:
    EntityList* m_list;
    uint32_t m_id;

    Entity(EntityList* list, uint32_t id) : m_list(list), m_id(id) {}

public:
    std::string& id();
    const std::string& id() const;
    std::string& name();
    const std::string& name() const;
    std::unordered_set<std::string>& tags();
    const std::unordered_set<std::string>& tags() const;

    sfu::AlphaMap& bitmap();
    const sfu::AlphaMap& bitmap() const;
    sfu::AnimatedSprite& sprite();
    const sfu::AnimatedSprite& sprite() const;
    sfu::AnimatedSprite& outline_sprite();
    const sfu::AnimatedSprite& outline_sprite() const;

    nav::Agent& tracker();
    const nav::Agent& tracker() const;
    SortBoundary& boundary();
    const SortBoundary& boundary() const;
    sfu::FloatCircle& collider();
    const sfu::FloatCircle& collider() const;

    std::string& dialogue();
    const std::string& dialogue() const;
    std::string& examination();
    const std::string& examination() const;
    std::vector<Action>& actions();
    const std::vector<Action>& actions() const;
    std::deque<shmy::sim::Event>& action_queue();
    const std::deque<shmy::sim::Event>& action_queue() const;

    EntityStats& stats();
    const EntityStats& stats() const;
    Inventory& inventory();
    const Inventory& inventory() const;
    uint32_t& flags();
    const uint32_t& flags() const;

    bool is_playable() const;
    bool is_offstage() const;
    bool is_hovered() const;
    bool is_ghost() const;

    friend class EntityList;
};


class EntityList {
public:
    std::vector<std::string> id;
    std::vector<std::string> name;
    std::vector<std::unordered_set<std::string>> tags;

    std::vector<sfu::AlphaMap> bitmap;
    std::vector<sfu::AnimatedSprite> sprite;
    std::vector<sfu::AnimatedSprite> outline_sprite;

    std::vector<nav::Agent> tracker;
    std::vector<SortBoundary> boundary;
    std::vector<sfu::FloatCircle> collider;

    std::vector<std::string> dialogue;
    std::vector<std::string> examination;
    std::vector<std::vector<Entity::Action>> actions;
    std::vector<std::deque<shmy::sim::Event>> action_queue;

    std::vector<EntityStats> stats;
    std::vector<Inventory> inventory;
    std::vector<uint32_t> flags;

    Entity get(uint32_t _id) { return Entity(this, _id); }
};


}
