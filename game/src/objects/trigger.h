#pragma once
#include <SFML/Graphics.hpp>
#include <string>
#include <variant>
#include "scripting/flag_expr.h"
#include "sfutil/geometry.h"


struct BeginScript   { std::string filename; };
struct BeginDialogue { std::string filename; };
struct Popup         { std::string message; };
struct GotoRegion    { std::string filename; };
struct GotoArea      { size_t index; sf::Vector2f spawnpos; bool suppress_triggers; };
using TriggerAction = std::variant<BeginScript, BeginDialogue, Popup, GotoRegion, GotoArea>;

struct Trigger {
    std::string id;
    std::string once_id;
    sfu::RotatedFloatRect bounds;
    TriggerAction action;
    FlagExpr condition = FlagExpr::True();
    bool single_use = false;
    bool cooldown = false;
};

