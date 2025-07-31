#pragma once
#include <SFML/Graphics.hpp>


namespace shmy { namespace filter {

sf::Image outline_threaded(const sf::Image& tex, int width);
sf::Image outline(const sf::Image& tex, int width);

sf::Image clickmap(const sf::Image& tex, int width);
sf::Image clickmap_threaded(const sf::Image& tex, int width);

// sf::Image map_area_threaded(const sf::Image& map, int avoid_radius);
// sf::Image map_area(const sf::Image& map, int avoid_radius);

} }

