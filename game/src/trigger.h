#pragma once
#include <SFML/Graphics.hpp>
#include <string>
#include <variant>


struct BeginScript { std::string filename; };
struct GotoRegion { std::string filename; };
struct GotoArea { size_t index; };
using TriggerAction = std::variant<BeginScript, GotoRegion, GotoArea>;

struct Trigger {
    std::string id;
    sf::FloatRect bounds;
    TriggerAction action;
    bool single_use = false;
    bool cooldown = false;
};

