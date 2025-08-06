#pragma once
#include <SFML/Graphics.hpp>
#include <rapidjson/document.h>
#include <sfutil/camera.h>
#include <navmesh/lib.h>
#include <lua/lua.hpp>
#include <unordered_map>
#include "graphics/background.h"
#include "gui/panel.h"
#include "objects/entity.h"
#include "objects/trigger.h"
#include "scripting/lua/script.h"
#include "game/dialogue.h"
#include "action.h"
#include "debugger.h"
#include "game/game_mode.h"


class Region;

struct Area {
    static const sf::RenderWindow* window;

    Region* p_region;
    std::string id;
    std::string area_label;

    shmy::AsyncBackground background;
    nav::NavMesh pathfinder;
    sf::Vector2f topleft;
    float scale;
    float zoom = 1.f;
    float zoom_target = 1.f;
    sf::Color overlaycolor;

    sf::Transform cart_to_iso;
    sf::Transform iso_to_cart;

    std::unordered_map<std::string, Entity> entities;
    std::unordered_map<std::string, std::string> script_name_LUT;
    std::unordered_map<std::string, std::string> story_name_LUT;
    std::vector<Entity*> sorted_entities;
    std::string player_id = "";

    lua_State* lua_vm;
    std::vector<shmy::lua::Script> scripts;
    std::vector<Trigger> triggers;
    bool suppress_triggers = false;
    bool suppress_portals = false;

    sf::RectangleShape motionguide_square;
    std::optional<ContextAction> queued;

    sfu::Camera camera;
    gui::Panel gui;

    GameMode gamemode = GameMode::Normal;
    NormalMode normal_mode;
    CinematicMode cinematic_mode;
    CombatMode combat_mode;
    SleepMode sleep_mode;

    sf::RectangleShape cinemabar_top;
    sf::RectangleShape cinemabar_bot;
    float cinematic_timer = 0.f;
    Dialogue dialogue;

#ifdef DEBUG
    AreaDebugView debugger;
#endif

    Area(const std::string& id, Region* parent_region);
    Area(const Area& other) = delete;
    Area(Area&& other);
    ~Area();

    void load_prefab(const rapidjson::Value& prefabs, const rapidjson::Value& value, const std::string& name);
    void load_entity(const rapidjson::Value& prefabs, const rapidjson::Value& value);
    void load_gui();

    void init(const rapidjson::Value& prefabs, const rapidjson::Document& doc);
    void set_mode(GameMode mode);

    Entity& get_player();
    const Entity& get_player() const;

    void update_motionguide();
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

