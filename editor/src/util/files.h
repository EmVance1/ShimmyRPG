#pragma once
#include <SFML/Graphics.hpp>
#include <filesystem>
#include <optional>


std::optional<std::filesystem::path> select_folder(sf::WindowHandle handle);

