#pragma once
#include <SFML/Graphics.hpp>


sf::Image gen_outline_threaded(const sf::Image& tex, int width);
sf::Image gen_outline(const sf::Image& tex, int width);

sf::Image map_area_threaded(const sf::Image& map, int avoid_radius);
sf::Image map_area(const sf::Image& map, int avoid_radius);

sf::Image gen_clickmap(const sf::Image& tex, int width);
sf::Image gen_clickmap_threaded(const sf::Image& tex, int width);
