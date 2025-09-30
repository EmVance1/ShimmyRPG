#pragma once
#include <SFML/Graphics.hpp>


sf::Transform cartesian_to_isometric(const sf::Vector2f& topleft);
sf::Transform isometric_to_cartesian(const sf::Vector2f& topleft);

