#pragma once
#include <SFML/Graphics.hpp>
#include "gui/gui.h"
#include "game/dialogue.h"


class Region;
struct Area;

class CinematicMode {
private:
    Region* p_region = nullptr;

    Area& get_area();

public:
    std::shared_ptr<gui::Panel> dia_gui;
    Dialogue dialogue;

public:
    CinematicMode() = default;

    void init(Region* _region) { p_region = _region; }

    void handle_event(const sf::Event& event);
    void update();
};

