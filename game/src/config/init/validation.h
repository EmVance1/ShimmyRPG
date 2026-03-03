#pragma once
#include <SFML/Graphics.hpp>
#include <rapidjson/document.h>
#include <vector>


struct PortalConfig {
    const char* id;
    sf::Vector2f position;
    const char* exit;
    const char* icon;
    float mouseover_radius;
    float preload_radius;
    bool locked;
};

struct TriggerConfig {
};

struct EntityConfig {
    enum { Character, Prop } type;
    const char* id;
    enum { World, Iso } space;
    sf::Vector2f position;
};

struct SceneConfig {
    const char* label;
    float zoom = 1.f;
    std::vector<const char*> scripts;
    std::vector<const char*> tracks;
    std::vector<PortalConfig> portals;
    std::vector<TriggerConfig> triggers;
    std::vector<EntityConfig> entities;

    struct Error {
        enum { Missing, WrongType } type;
        std::string value;
        bool recoverable;
    };
    std::vector<Error> errors;
};

SceneConfig validate_scene_config(const rapidjson::Value& v);

