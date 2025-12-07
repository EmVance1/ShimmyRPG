#pragma once
#include <SFML/Graphics.hpp>
#include <string>
#include <variant>
#include "scripting/expr.h"
#include "sfutil/geometry.h"


namespace action {

struct DoString { std::string str;  };
struct DoEvent  { std::string event; };
struct LoadDia  { std::string file; };
struct Popup    { std::string msg;  };
struct Portal   { size_t index; sf::Vector2f spawnpos; std::string lock_id; };

}

using TriggerAction = std::variant<action::DoString, action::DoEvent, action::LoadDia, action::Popup, action::Portal>;

struct Trigger {
    sfu::RotatedFloatRect bounds;
    TriggerAction action;
    bool cooldown = false;
    bool used = false;
    shmy::Expr condition = shmy::Expr::True();
};

