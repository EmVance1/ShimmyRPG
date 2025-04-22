#pragma once
#include <SFML/Graphics.hpp>
#include <unordered_map>
#include <rapidjson/document.h>
#include <sfutil/camera.h>
#include "action.h"
#include "algo/graph2d.h"
#include "gui/panel.h"
#include "graphics/partialline.h"
#include "entity.h"
#include "trigger.h"
#include "debugger.h"
#include "scripts/lua_script.h"
#include "scripts/dialogue.h"

#include "game/game_mode.h"


class Region;

struct Area {
    static const sf::RenderWindow* window;

    Region* p_region;
    const std::string id;
    std::string area_label;

    SpatialGraph2d pathfinder;
    sf::Vector2f topleft;
    sf::Sprite background;
    float scale;

    const sf::Transform cart_to_iso;
    const sf::Transform iso_to_cart;

    std::unordered_map<std::string, Entity> entities;
    std::unordered_map<std::string, std::string> script_name_LUT;
    std::unordered_map<std::string, std::string> dialogue_name_LUT;
    std::vector<Entity*> sorted_entities;
    std::string player_id;

    std::vector<Trigger> triggers;
    bool suppress_triggers = false;
    std::vector<LuaScript> scripts;

    sf::RectangleShape motionguide_square;
    PartialLine motionguide_line;
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

    Area(const std::string& id, Region* parent_region, const sf::Vector2f& topleft, float pathscale);

    void init(const rapidjson::Document& doc);
    void set_mode(GameMode mode, bool dramatic);

    void set_player_position(const sf::Vector2f& position, bool suppress_triggers);
    void sort_sprites();
    Entity& get_player();
    void update_motionguide();
    void handle_trigger(const Trigger& trigger);
    void begin_dialogue(const SpeechGraph& graph);

    void handle_event(const sf::Event& event);
    void update();
    void render(sf::RenderTarget& target);
};

