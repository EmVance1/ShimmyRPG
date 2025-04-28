#pragma once
#include <SFML/Graphics.hpp>
#include <rapidjson/document.h>
#include "algo/allocators.h"


class Background : public sf::Drawable {
private:
    static FreeListAllocator<sf::Texture> ALLOC;

    struct Tile {
        sf::Texture* texture = nullptr;
        std::string filename;
        sf::FloatRect bounds;
    };

    std::vector<Tile> m_tiles;

public:
    Background() = default;
    ~Background();

    void load_from_json(const rapidjson::Value& value);

    void update(const sf::FloatRect& frustum);
    void draw(sf::RenderTarget& target, sf::RenderStates states) const;
};

