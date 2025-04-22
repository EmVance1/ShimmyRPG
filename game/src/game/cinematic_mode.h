#pragma once
#include <SFML/Graphics.hpp>


struct Area;

class CinematicMode {
private:
    Area* p_area = nullptr;

public:
    CinematicMode() = default;

    void init(Area* area) { p_area = area; }

    void handle_event(const sf::Event& event);
    void update();
};

