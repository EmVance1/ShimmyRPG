#pragma once

#include <SFML/Graphics.hpp>

#include <json.hpp>

#include <iostream>
#include <fstream>
#include <sstream>

#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <string>

#include "imgui/imgui.h"
#include "imgui/imgui-SFML.h"
#include "imgui/imgui_stdlib.h"
#include "imgui/imconfig.h"

template<typename T>
std::ostream& operator<<(std::ostream& stream, const sf::Vector2<T>& v) {
    return stream << v.x << ", " << v.y;
}

