#pragma once
#include <SFML/Graphics.hpp>
#include <rapidjson/document.h>


class Background : public sf::Drawable {
private:
    struct Tile {
        sf::Texture texture;
        std::string filename;
        sf::FloatRect bounds;
        bool loaded;
    };

    std::vector<Tile> m_tiles;

public:
    Background() = default;

    void load_from_json(const rapidjson::Value& value);

    void update(const sf::FloatRect& frustum);
    void draw(sf::RenderTarget& target, sf::RenderStates states) const;
};

