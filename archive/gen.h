#pragma once
#include <SFML/Graphics.hpp>


class Map {
private:
    std::vector<std::vector<bool>> m_walls;

    const inline static sf::Vector2u LUT[] = {
        {11, 0 }, { 3, 1 }, { 0, 1 }, { 0, 1 }, { 3, 2 }, { 4, 1 }, { 0, 1 }, { 0, 1 },
        { 0, 0 }, { 0, 0 }, { 1, 0 }, { 1, 0 }, { 6, 0 }, { 6, 0 }, { 1, 0 }, { 1, 0 },

        { 0, 2 }, { 7, 2 }, { 1, 1 }, { 1, 1 }, { 0, 2 }, { 7, 2 }, { 1, 1 }, { 1, 1 },
        {10, 2 }, {10, 2 }, { 2, 0 }, { 2, 0 }, {10, 2 }, {10, 2 }, { 2, 0 }, { 2, 0 },

        { 3, 0 }, { 4, 0 }, { 7, 1 }, { 7, 1 }, {11, 3 }, { 5, 0 }, { 7, 1 }, { 7, 1 },
        { 0, 0 }, { 0, 0 }, { 1, 0 }, { 1, 0 }, { 6, 0 }, { 6, 0 }, { 1, 0 }, { 1, 0 },

        { 6, 2 }, { 8, 2 }, { 9, 1 }, { 9, 1 }, { 6, 2 }, { 8, 2 }, { 9, 1 }, { 9, 1 },
        {10, 2 }, {10, 2 }, { 2, 0 }, { 2, 0 }, {10, 2 }, {10, 2 }, { 2, 0 }, { 2, 0 },

        { 0, 3 }, { 6, 3 }, {10, 3 }, {10, 3 }, { 7, 3 }, { 8, 3 }, {10, 3 }, {10, 3 },
        { 1, 3 }, { 1, 3 }, { 2, 3 }, { 2, 3 }, { 9, 3 }, { 9, 3 }, { 2, 3 }, { 2, 3 },

        { 1, 2 }, { 9, 2 }, { 2, 1 }, { 2, 1 }, { 1, 2 }, { 9, 2 }, { 2, 1 }, { 2, 1 },
        { 2, 2 }, { 2, 2 }, {10, 0 }, {10, 0 }, { 2, 2 }, { 2, 2 }, {10, 0 }, {10, 0 },

        { 0, 3 }, { 6, 3 }, {10, 3 }, {10, 3 }, { 7, 3 }, { 8, 3 }, {10, 3 }, {10, 3 },
        { 1, 3 }, { 1, 3 }, { 2, 3 }, { 2, 3 }, { 9, 3 }, { 9, 3 }, { 1, 3 }, { 1, 3 },

        { 1, 2 }, { 9, 2 }, { 2, 1 }, { 2, 1 }, { 1, 2 }, { 9, 2 }, { 2, 1 }, { 2, 1 },
        { 2, 2 }, { 2, 2 }, {10, 0 }, {10, 0 }, { 2, 2 }, { 2, 2 }, {10, 0 }, {10, 0 },

        // TODO 128+
    };

public:
    Map();

    sf::Vector2u getFlat(const sf::Vector2u& wall) const;
    sf::Vector2u getTileIndex(const sf::Vector2u& wall) const;
};


void print_all_modes();

