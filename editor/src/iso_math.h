#pragma once
#include <SFML/Graphics.hpp>
#include "repr.h"


sf::Transform cartesian_to_isometric(const sf::Vector2f& origin);
sf::Transform isometric_to_cartesian(const sf::Vector2f& origin);

sf::Transform grid_to_cartesian(float scale);
sf::Transform cartesian_to_grid(float scale);

sf::Transform grid_to_isometric(const sf::Vector2f& origin, float scale);
sf::Transform isometric_to_grid(const sf::Vector2f& origin, float scale);

Position map_spaces(const Position& from, Position::Mode to, const sf::Vector2f& origin, float scale);

sf::Image undo_isometric(const sf::Image& img);
sf::Texture undo_isometric(const sf::Texture& img);

