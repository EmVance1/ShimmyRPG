#pragma once
#include <SFML/Graphics.hpp>
#include "repr.h"


sf::Transform cart_to_iso(const sf::Vector2f& origin, float scale);
sf::Transform iso_to_cart(const sf::Vector2f& origin, float scale);

Position map_spaces(const Position& from, Position::Mode to, const sf::Vector2f& origin, float scale);

