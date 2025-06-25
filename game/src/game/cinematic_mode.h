#pragma once
#include <SFML/Graphics.hpp>
#include "gui/gui.h"


struct Area;

class CinematicMode {
private:
    Area* p_area = nullptr;

public:
    std::shared_ptr<gui::Panel> dia_gui;

public:
    CinematicMode() = default;

    void init(Area* area) { p_area = area; }

    void handle_event(const sf::Event& event);
    void update();
};

