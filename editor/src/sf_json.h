#pragma once
#include <SFML/Graphics.hpp>
#include <json.hpp>

namespace nm = nlohmann;


sf::Vector2f json_to_vector2f(const nm::json& arr);
sf::Vector2i json_to_vector2i(const nm::json& arr);
sf::Vector2u json_to_vector2u(const nm::json& arr);

sf::FloatRect json_to_floatrect(const nm::json& arr);
sf::IntRect   json_to_intrect  (const nm::json& arr);

sf::Color json_to_color(const nm::json& arr);


nm::json vector2f_to_json(const sf::Vector2f& val);
nm::json vector2i_to_json(const sf::Vector2i& val);
nm::json vector2u_to_json(const sf::Vector2u& val);

nm::json floatrect_to_json(const sf::FloatRect& val);
nm::json intrect_to_json  (const sf::IntRect&   val);

nm::json color_to_json(const sf::Color& val);

