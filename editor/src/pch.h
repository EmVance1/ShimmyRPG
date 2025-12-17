#pragma once

#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <string>

#include <filesystem>
#include <iostream>
#include <fstream>
#include <sstream>

#include <SFML/Graphics.hpp>
#include <json.hpp>

#include <imgui.h>
#include <imgui-SFML.h>
#include <imgui_stdlib.h>
#include <imconfig.h>

#include <navmesh/lib.h>

template<typename T>
std::ostream& operator<<(std::ostream& stream, const sf::Vector2<T>& v) {
    return stream << "(" << v.x << ", " << v.y << ")";
}
template<typename T>
std::ostream& operator<<(std::ostream& stream, const sf::Rect<T>& v) {
    return stream << "[(" << v.position.x << ", " << v.position.y << "), " << "(" << v.size.x << ", " << v.size.y << ")]";
}

