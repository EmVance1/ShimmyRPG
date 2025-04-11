#include "pch.h"
#include "gen.h"


Map::Map() {
    m_walls = {
        { 1, 1, 1, 1, 1, 0, 1, 1, 1, 1 },
        { 1, 1, 0, 0, 1, 0, 0, 0, 0, 1 },
        { 1, 0, 0, 1, 1, 0, 0, 1, 0, 1 },
        { 1, 0, 0, 1, 1, 0, 0, 1, 0, 1 },
        { 1, 1, 0, 0, 1, 0, 0, 0, 0, 1 },
        { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 },
        { 1, 0, 1, 0, 1, 0, 0, 1, 0, 1 },
        { 1, 0, 1, 0, 1, 0, 0, 1, 0, 1 },
        { 1, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
        { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 },
    };
}

sf::Vector2u Map::getFlat(const sf::Vector2u& wall) const {
    const auto actual = sf::Vector2u(wall.x, wall.y);
    if (m_walls[actual.y][actual.x]) {
        return sf::Vector2u(11, 1);
    } else {
        return sf::Vector2u(11, 0);
    }
}

sf::Vector2u Map::getTileIndex(const sf::Vector2u& wall) const {
    if (m_walls[wall.y][wall.x]) {
        return sf::Vector2u(11, 1);
    }

    uint8_t index = 0;
    index |= m_walls[wall.y - 1][wall.x - 1] << 0;
    index |= m_walls[wall.y - 1][wall.x    ] << 1;
    index |= m_walls[wall.y - 1][wall.x + 1] << 2;
    index |= m_walls[wall.y    ][wall.x - 1] << 3;
    index |= m_walls[wall.y    ][wall.x + 1] << 4;
    index |= m_walls[wall.y + 1][wall.x - 1] << 5;
    index |= m_walls[wall.y + 1][wall.x    ] << 6;
    // index |= m_walls[wall.y + 1][wall.x + 1] << 7;
    return LUT[index];
}


void print_all_modes() {
    for (int i = 0; i < 256; i++) {
        std::cout << i << ":\n    ";
        if (i & (1 << 0)) std::cout << "#"; else std::cout << "_";
        if (i & (1 << 1)) std::cout << "#"; else std::cout << "_";
        if (i & (1 << 2)) std::cout << "#"; else std::cout << "_";
        std::cout << "\n    ";
        if (i & (1 << 3)) std::cout << "#"; else std::cout << "_";
        std::cout << "_";
        if (i & (1 << 4)) std::cout << "#"; else std::cout << "_";
        std::cout << "\n    ";
        if (i & (1 << 5)) std::cout << "#"; else std::cout << "_";
        if (i & (1 << 6)) std::cout << "#"; else std::cout << "_";
        if (i & (1 << 7)) std::cout << "#"; else std::cout << "_";
        std::cout << "\n";
    }
}


