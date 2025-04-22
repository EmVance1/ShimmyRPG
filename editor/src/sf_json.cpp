#include "pch.h"
#include "sf_json.h"


sf::Vector2f json_to_vector2f(const nm::json& arr) {
    return sf::Vector2f(arr[0], arr[1]);
}

sf::Vector2i json_to_vector2i(const nm::json& arr) {
    return sf::Vector2i(arr[0], arr[1]);
}

sf::Vector2u json_to_vector2u(const nm::json& arr) {
    return sf::Vector2u(arr[0], arr[1]);
}


sf::FloatRect json_to_floatrect(const nm::json& arr) {
    return sf::FloatRect({ arr[0], arr[1] }, { arr[2], arr[3] });
}

sf::IntRect json_to_intrect(const nm::json& arr) {
    return sf::IntRect({ arr[0], arr[1] }, { arr[2], arr[3] });
}


sf::Color json_to_color(const nm::json& arr) {
    if (arr.size() == 3) {
        return sf::Color((uint8_t)arr[0],
                         (uint8_t)arr[1],
                         (uint8_t)arr[2]);
    } else {
        return sf::Color((uint8_t)arr[0],
                         (uint8_t)arr[1],
                         (uint8_t)arr[2],
                         (uint8_t)arr[3]);
    }
}


nm::json vector2f_to_json(const sf::Vector2f& val) {
    auto result = nm::json();
    result.push_back(val.x);
    result.push_back(val.y);
    return result;
}

nm::json vector2i_to_json(const sf::Vector2i& val) {
    auto result = nm::json();
    result.push_back(val.x);
    result.push_back(val.y);
    return result;
}

nm::json vector2u_to_json(const sf::Vector2u& val) {
    auto result = nm::json();
    result.push_back(val.x);
    result.push_back(val.y);
    return result;
}


nm::json floatrect_to_json(const sf::FloatRect& val) {
    auto result = nm::json();
    result.push_back(val.position.x);
    result.push_back(val.position.y);
    result.push_back(val.size.x);
    result.push_back(val.size.y);
    return result;
}

nm::json intrect_to_json  (const sf::IntRect& val) {
    auto result = nm::json();
    result.push_back(val.position.x);
    result.push_back(val.position.y);
    result.push_back(val.size.x);
    result.push_back(val.size.y);
    return result;
}


nm::json color_to_json(const sf::Color& val) {
    auto result = nm::json();
    result.push_back(val.r);
    result.push_back(val.g);
    result.push_back(val.b);
    result.push_back(val.a);
    return result;
}

