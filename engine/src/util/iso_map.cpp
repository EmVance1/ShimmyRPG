#include "pch.h"


sf::Transform world_to_screen(const sf::Vector2f& origin, float scale, float angle) {
    auto t = sf::Transform();
    t.translate(origin);
    t.scale({std::sqrt(2.f) * scale, scale});
    t.rotate(sf::degrees(angle));
    return t;
}

sf::Transform screen_to_world(const sf::Vector2f& origin, float scale, float angle) {
    return world_to_screen(origin, scale, angle).getInverse();
}

