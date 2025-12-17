#pragma once
#include <SFML/Graphics.hpp>


sf::Transform world_to_screen(const sf::Vector2f& origin, float scale, float angle = 45);
sf::Transform screen_to_world(const sf::Vector2f& origin, float scale, float angle = 45);

