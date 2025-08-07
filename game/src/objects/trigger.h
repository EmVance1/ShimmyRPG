#pragma once
#include <SFML/Graphics.hpp>
#include <string>
#include <variant>
#include "flags.h"
#include "scripting/flag_expr.h"
#include "sfutil/geometry.h"


struct BeginScript   { std::string filename; };
struct BeginDialogue { std::string filename; };
struct Popup         { std::string message; };
struct GotoRegion    { std::string filename; };
struct GotoArea      { size_t index; sf::Vector2f spawnpos; std::string lock_id; };
struct CameraZoom    { float target; };
struct ChangeFlag    { std::string name; FlagModifier mod; };
using TriggerAction = std::variant<BeginScript, BeginDialogue, Popup, GotoRegion, GotoArea, CameraZoom, ChangeFlag>;

struct Trigger {
    sfu::RotatedFloatRect bounds;
    TriggerAction action;
    std::string once_id;
    shmy::FlagExpr condition = shmy::FlagExpr::True();
    bool cooldown = false;
};

