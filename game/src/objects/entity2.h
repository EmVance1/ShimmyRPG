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
    enum class Type {
        Character,
        Mob,
        Prop,
        Scenery,
    };
    enum Flag {
        Loaded   = 1,
        Playable = 1 << 1,
        Offstage = 1 << 2,
        Hovered  = 1 << 3,
        Ghost    = 1 << 4,
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

    const sfu::AlphaMap& bitmap() const;
    sfu::AnimatedSprite& sprite();
    const sfu::AnimatedSprite& sprite() const;
    sfu::AnimatedSprite& sprite_outline();
    const sfu::AnimatedSprite& sprite_outline() const;

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

    bool is_loaded() const;
    bool is_playable() const;
    bool is_offstage() const;
    bool is_hovered() const;
    bool is_ghost() const;

    friend class EntityList;
};


template<typename T>
struct Asset {
    enum LoadState { Unloaded, Loading, Loaded } state;
    T object;
    size_t refcount;

    void load()       { state = LoadState::Loaded; }
    void unload()     { state = LoadState::Unloaded; }
    void async_load() { state = LoadState::Loading; }
};
template<typename T>
class AssetRef {
    Asset<T>* ref;

public:
    AssetRef(Asset<T>* ptr) : ref(ptr) {}

    const T& get() const { ref->load(); return ref->object; }
    const std::optional<T&> get_opt() const {
        if (ref->state == Asset<T>::LoadState::Loaded) {
            return ref->object;
        } else {
            return {};
        }
    }
    void release() { ref->release(); }
};
template<typename T>
class AssetHandle {
private:
    Asset<T>* ref;

public:
    AssetRef<T> prepare() {
        if (ref->refcount == 0) ref->async_load();
        ref->refcount++;
        return { ref };
    }
    AssetRef<T> acquire() {
        if (ref->refcount == 0) ref->load();
        ref->refcount++;
        return { ref };
    }
    void release() {
        ref->refcount--;
        if (ref->refcount == 0) ref->unload();
    }

    // no clean invariants for me
    const T& get() const {
        if (ref->state == Asset<T>::LoadState::Loading) {
            ref->load();
        }
        return ref->object;
    }
    const std::optional<T&> get_opt() const {
        if (ref->state == Asset<T>::LoadState::Loaded) {
            return ref->object;
        } else {
            return {};
        }
    }
};

class EntityList {
public:
    size_t n_npc;
    size_t n_mob;
    size_t n_prop;
    size_t n_scenery;

public:
    std::vector<std::string> id;
    std::vector<std::string> name;
    std::vector<std::unordered_set<std::string>> tags;

    std::vector<AssetHandle<sfu::AlphaMap>> bitmap;
    std::vector<AssetHandle<sfu::TextureAtlas>> anim;
    std::vector<AssetHandle<sfu::TextureAtlas>> anim_outline;

    std::vector<sfu::AnimatedSprite> sprite;
    std::vector<sfu::AnimatedSprite> sprite_outline;

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
    void load(uint32_t _id);
    void unload(uint32_t _id);
};


}
