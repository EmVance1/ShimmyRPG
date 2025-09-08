#pragma once
#include <SFML/Graphics.hpp>
#include <lua/lua.hpp>
#include <string>
#include <vector>
#include <navmesh/types.h>
#include <sfutil/camera.h>
#include "graphics/background.h"
#include "scripting/lua/script.h"
#include "gui/gui.h"
#include "objects/trigger.h"
#include "objects/entity.h"


class Region;

struct Scene {
    static const sf::RenderWindow* window;

    Region* p_region;
    std::string id;
    std::string area_label;

    shmy::AsyncBackground background;
    nav::NavMesh pathfinder;
    sf::Vector2f topleft;
    float scale;

    std::unordered_map<std::string, Entity> entities;
    std::unordered_map<std::string, std::string> script_to_UUID;
    std::unordered_map<std::string, std::string> story_to_UUID;
    std::vector<Entity*> sorted_entities; // to be removed in favour of z sorting
    std::string player_uuid = "";

    lua_State* lua_vm;
    std::vector<shmy::lua::Script> scripts;
    std::vector<Trigger> triggers;

    sfu::Camera camera;
    gui::Panel gui;

    GameMode gamemode = GameMode::Normal;
    NormalMode normal_mode;
    CinematicMode cinematic_mode;
    CombatMode combat_mode;
    SleepMode sleep_mode;

#ifdef VANGO_DEBUG
    SceneDebugger debugger;
#endif

    Scene(const std::string& id, Region* parent_region);
    Scene(const Scene& other) = delete;
    Scene(Scene&& other);
    ~Scene();

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

