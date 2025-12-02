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

struct Scene {
    static RenderSettings* render_settings;

    Region* region;

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

    bool awake = false;

    Scene();
    Scene(const Scene& other) = delete;
    Scene(Scene&& other);

    const Entity& get_player() const;
    Entity& get_player();

    Entity& get_entity_by_script_id(const std::string& id);
    const Entity& get_entity_by_script_id(const std::string& id) const;

    void handle_event(const sf::Event& event);
    void update();
    void render(sf::RenderTarget& target);
};

