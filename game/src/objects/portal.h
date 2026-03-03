#pragma once
#include <SFML/Graphics.hpp>
#include <string>
#include "gui/gui.h"


struct Portal {
    sf::Vector2f position;
    size_t exit_scene;
    std::string exit_portal;
    float mouseover_radius;
    float preload_radius;
    bool locked = false;

    std::shared_ptr<gui::Button> icon;
    // "icon": "nav_icons.door",
};

