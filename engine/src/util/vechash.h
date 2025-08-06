#pragma once
#include <SFML/Graphics.hpp>


namespace std {
    template<typename T>
    class hash<sf::Vector2<T>> {
    public:
        std::uint64_t operator()(const sf::Vector2<T>& v) const {
            return (53 + std::hash<T>()(v.y)) * 53 + std::hash<T>()(v.x);
        }
    };
}

