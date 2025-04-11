#include "pch.h"


sf::Transform cartesian_to_isometric(const sf::Vector2f& topleft) {
    auto t = sf::Transform();
    t.translate(topleft);
    t.scale({std::sqrt(2.f), 1.f});
    t.rotate(sf::degrees(45));
    return t;
}

sf::Transform isometric_to_cartesian(const sf::Vector2f& topleft) {
    auto t = sf::Transform();
    t.rotate(sf::degrees(-45));
    t.scale({1.f / std::sqrt(2.f), 1.f});
    t.translate(-topleft);
    return t;
}

