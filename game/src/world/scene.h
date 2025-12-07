#pragma once
#include <SFML/Graphics.hpp>
#include <rapidjson/document.h>
#include <luajit-2.1/lua.hpp>
#include <sfutil/camera.h>
#include <navmesh/lib.h>
#include <unordered_map>
#include "scripting/lua/runtime.h"
#include "graphics/background.h"
#include "objects/entity.h"
#include "objects/trigger.h"


class Region;

struct Scene {
    Region* region;
    std::string name;

    shmy::AsyncBackground background;
    nav::Mesh pathfinder;
    sf::Vector2f topleft;
    sfu::Camera camera;

    shmy::lua::Runtime runtime;
    std::vector<Trigger> triggers;
    std::unordered_map<std::string, Entity> entities;
    std::unordered_map<std::string, std::string> script_to_UUID;
    std::vector<Entity*> sorted_entities; // to be removed in favour of z sorting
    std::string player_uuid = "";

    bool sleeping = true;

    Scene();
    Scene(const Scene& other) = delete;
    Scene(Scene&& other);

    const Entity& get_player() const;
    Entity& get_player();

    Entity& get_entity_by_script_id(const std::string& id);
    const Entity& get_entity_by_script_id(const std::string& id) const;

    void set_sleeping(bool sleeping);

    void handle_event(const sf::Event& event);
    void update();
    void render(sf::RenderTarget& target);
};

