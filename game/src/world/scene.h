#pragma once
#include <SFML/Graphics.hpp>
#include <sfutil/camera.h>
#include <navmesh/lib.h>
#include <unordered_map>
#include "graphics/background.h"
#include "scripting/lua/runtime.h"
#include "scripting/speech/graph.h"
#include "objects/entity.h"
#include "objects/trigger.h"
#include "game/rules/game_mode.h"
#include "game/action.h"
#ifdef SHMY_DEBUG
#include "debugger.h"
#endif


class Game;
class SceneLoader;

struct Scene {
    Game* game;
    std::string name;

    // environment
    nav::Mesh pathfinder;
    shmy::BackgroundStream background;
    sf::Vector2f origin;
    float scale;
    sfu::Camera camera;
    sf::Transform world_to_screen;
    sf::Transform screen_to_world;
    std::unordered_set<std::string> refs;

    // entities, scripts, triggers
    shmy::lua::Runtime lua_vm;
    std::vector<Trigger> triggers;
    std::unordered_map<std::string, Entity> entities;
    std::unordered_map<std::string, std::string> script_to_uuid;
    std::vector<Entity*> sorted_entities;
    std::string player_id = "";

    // misc data
    sf::RectangleShape motionguide_square;
    std::optional<ContextAction> queued;
    bool sleeping = true;

#ifdef SHMY_DEBUG
    SceneDebugger debugger;
#endif

    Scene(SceneLoader& loader, const std::string& r_id, const std::string& s_id);
    Scene(const Scene& other) = delete;
    Scene(Scene&& other) noexcept;
    ~Scene();

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

    void set_mode(GameMode mode);
    void set_sleeping(bool sleeping);

    void handle_event(const sf::Event& event);
    void update();
    void render(sf::RenderTarget& target);
};

