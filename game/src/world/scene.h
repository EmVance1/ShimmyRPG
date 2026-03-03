#pragma once
#include <SFML/Graphics.hpp>
#include <sfutil/camera.h>
#include <navmesh/lib.h>
#include <unordered_set>
#include "graphics/background.h"
#include "scripting/lua/runtime.h"
#include "objects/portal.h"
#include "objects/trigger.h"
#include "game/event.h"


class Game;
class Debugger;
class SceneLoader;

struct Scene {
    Game* game;
    std::string name;

    // environment
    shmy::BackgroundStream background;
    nav::Mesh pathfinder;
    sf::Vector2f origin;
    float scale;
    sfu::Camera camera;
    sf::Transform world_to_screen;
    sf::Transform screen_to_world;

    // assets
    std::unordered_set<std::string> refs;
    std::unordered_set<std::string> tracks;

    // entities, scripts, triggers
    shmy::lua::Runtime lua_vm;
    std::unordered_map<std::string, Portal> portals;
    std::vector<Trigger> triggers;
    std::vector<uint32_t> entities;

    // misc data
    sf::RectangleShape motionguide_square;
    bool sleeping = true;

    Scene(SceneLoader& loader, const std::string& r_id, const std::string& s_id);
    Scene(const Scene& other) = delete;
    Scene(Scene&& other) noexcept;
    ~Scene();

    uint32_t sprites_top(const sf::Vector2f& p);
    void sprites_sort();

    void set_sleeping(bool sleeping);
    void init_gui(gui::Panel& root);

    void handle_trigger(const Trigger& trigger, uint32_t entity);
    void handle_event(const shmy::Event& event);
    void handle_input(const sf::Event& event);
    void update();
    void render(sf::RenderTarget& target, const Debugger* debugger);
};

