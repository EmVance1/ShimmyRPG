#pragma once
#include <SFML/Graphics.hpp>
#include <functional>


sf::Time benchmark_function(std::function<void()> f);

