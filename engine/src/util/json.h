#pragma once
#include <SFML/Graphics.hpp>
#include <rapidjson/rapidjson.h>
#include <rapidjson/document.h>


sf::Vector2f json_to_vector2f(const rapidjson::Value& arr);
sf::Vector2i json_to_vector2i(const rapidjson::Value& arr);
sf::Vector2u json_to_vector2u(const rapidjson::Value& arr);

sf::FloatRect json_to_floatrect(const rapidjson::Value& arr);
sf::IntRect json_to_intrect(const rapidjson::Value& arr);

sf::Color json_to_color(const rapidjson::Value& arr);

rapidjson::Document load_json_from_file(const std::string& filename);

