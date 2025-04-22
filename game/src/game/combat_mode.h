#pragma once
#include <SFML/Graphics.hpp>


struct Area;

class CombatMode {
private:
    Area* p_area = nullptr;

public:
    CombatMode() = default;

    void init(Area* area) { p_area = area; }

    void handle_event(const sf::Event& event);
    void update();
};

