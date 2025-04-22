#pragma once
#include <SFML/Graphics.hpp>
#include <string>
#include <variant>


struct BeginScript { std::string filename; };
struct Popup { std::string message; };
struct GotoRegion { std::string filename; };
struct GotoArea { size_t index; sf::Vector2f spawnpos; bool suppress_triggers; };
using TriggerAction = std::variant<BeginScript, Popup, GotoRegion, GotoArea>;

struct Trigger {
    std::string id;
    sf::FloatRect bounds;
    TriggerAction action;
    std::string condition = "1";
    bool single_use = false;
    bool cooldown = false;
};

