#pragma once
#include <SFML/Graphics.hpp>
#include <sfutil/camera.h>
#include <navmesh/lib.h>
#include <luajit-2.1/lua.hpp>
#include <unordered_map>
#include "graphics/background.h"
#include "scripting/lua/runtime.h"
#include "scripting/lua/script.h"
#include "gui/panel.h"
#include "objects/entity.h"
#include "objects/trigger.h"
#include "game/rules/game_mode.h"
#include "action.h"
#include "debugger.h"


struct RenderSettings {
    const sf::Vector2i viewport;
    std::vector<sf::Shader> shaders;
    sf::FloatRect crop;
    sf::Color overlay;

    RenderSettings(const sf::Vector2i& _viewport)
        : viewport(_viewport), crop({0, 0}, (sf::Vector2f)viewport), overlay(sf::Color::White)
    {}
};


class Region;

struct Area {
    static RenderSettings* render_settings;

    Region* region;
    std::string name;

    nav::Mesh pathfinder;
    shmy::AsyncBackground background;
    float zoom = 1.f;
    float zoom_target = 1.f;
    sf::Vector2f topleft;
    sf::Transform cart_to_iso;
    sf::Transform iso_to_cart;

    std::unordered_map<std::string, Entity> entities;
    std::unordered_map<std::string, std::string> script_to_uuid;
    std::vector<Entity*> sorted_entities;
    std::string player_id = "";

    shmy::lua::Runtime lua_vm;
    std::vector<Trigger> triggers;
    bool suppress_portals = false;

    sf::RectangleShape motionguide_square;
    std::optional<ContextAction> queued;

    sfu::Camera camera;
    gui::Panel gui;

    GameMode gamemode = GameMode::Normal;
    NormalMode normal_mode;
    CinematicMode cinematic_mode;
    CombatMode combat_mode;

#ifdef VANGO_DEBUG
    AreaDebugger debugger;
#endif

    Area();
    Area(const Area& other) = delete;
    Area(Area&& other);

    void set_mode(GameMode mode);

    Entity& get_player();
    const Entity& get_player() const;

    Entity& get_entity_by_script_id(const std::string& id);
    const Entity& get_entity_by_script_id(const std::string& id) const;

    void handle_trigger(const Trigger& trigger);

    void begin_dialogue(shmy::speech::Graph&& graph, const std::string& id);
    void begin_combat(
            const std::unordered_set<std::string>& ally_tags,
            const std::unordered_set<std::string>& enemy_tags,
            const std::unordered_set<std::string>& enemysenemy_tags,
            const std::unordered_set<std::string>& unaligned_tags
        );

    void handle_event(const sf::Event& event);
    void update();
    void render_world(sf::RenderTarget& target);
    void render_overlays(sf::RenderTarget& target);
};

