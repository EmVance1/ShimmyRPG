#pragma once
#include <SFML/Graphics.hpp>
#include <unordered_map>
#include <rapidjson/document.h>
#include <sfutil/camera.h>
#include "algo/graph2d.h"
#include "objects/entity.h"
#include "objects/trigger.h"
#include "scripts/lua_script.h"
#include "scripts/dialogue.h"
#include "action.h"
#include "gui/panel.h"
#include "background.h"
#include "debugger.h"

#include "game/game_mode.h"


class Region;

struct Area {
    static const sf::RenderWindow* window;

    Region* p_region;
    const std::string id;
    std::string area_label;
    sf::Shader posterize;

    Background background;
    SpatialGraph2d pathfinder;
    const sf::Vector2f topleft;
    const float scale;

    const sf::Transform cart_to_iso;
    const sf::Transform iso_to_cart;

    std::unordered_map<std::string, Entity> entities;
    std::unordered_map<std::string, std::string> script_name_LUT;
    std::unordered_map<std::string, std::string> story_name_LUT;
    std::vector<Entity*> sorted_entities;
    std::string player_id = "";

    std::vector<Trigger> triggers;
    bool suppress_triggers = false;
    std::vector<LuaScript> scripts;

    sf::RectangleShape motionguide_square;
    float motionguide_await = 11.f;
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

    Area(const std::string& id, Region* parent_region, const sf::Vector2f& topleft, float scale);

    void load_prefab(const rapidjson::Value& prefabs, const rapidjson::Value& value, const std::string& name);
    void load_entity(const rapidjson::Value& prefabs, const rapidjson::Value& value);
    void load_gui();

    void init(const rapidjson::Value& prefabs, const rapidjson::Document& doc);
    void set_mode(GameMode mode, bool dramatic);

    Entity& get_player();
    const Entity& get_player() const;

    void update_motionguide();
    void handle_trigger(const Trigger& trigger);
    void begin_dialogue(const SpeechGraph& graph, const std::string& id);

    void handle_event(const sf::Event& event);
    void update();
    void render(sf::RenderTarget& target);
};

