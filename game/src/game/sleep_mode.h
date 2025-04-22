#pragma once
#include <SFML/Graphics.hpp>


struct Area;

class SleepMode {
private:
    Area* p_area = nullptr;

public:
    SleepMode() = default;

    void init(Area* area) { p_area = area; }

    void handle_event(const sf::Event& event);
    void update();
};


