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
    bool m_dynamic;

public:
    Background(bool dynamic = false);

    void load_from_json(const rapidjson::Value& value);
    void prep_from_json(const rapidjson::Value& value);

    void load_textures();
    void unload_textures();

    void update(const sf::FloatRect& frustum);
    void draw(sf::RenderTarget& target, sf::RenderStates states) const;
};

