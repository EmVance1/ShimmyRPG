#include "pch.h"
#include "background.h"
#include "util/json.h"


Background::Background(bool dynamic) : m_dynamic(dynamic) {
}

void Background::load_from_json(const rapidjson::Value& value) {
    m_tiles.reserve(value.GetArray().Size());
    for (const auto& pair : value.GetArray()) {
        if (m_dynamic) {
            m_tiles.push_back(Tile{
                sf::Texture(),
                pair["file"].GetString(),
                json_to_floatrect(pair["bounds"]),
                false,
            });
        } else {
            m_tiles.push_back(Tile{
                sf::Texture(pair["file"].GetString()),
                pair["file"].GetString(),
                json_to_floatrect(pair["bounds"]),
                true,
            });
        }
    }
}

void Background::prep_from_json(const rapidjson::Value& value) {
    m_tiles.reserve(value.GetArray().Size());
    for (const auto& pair : value.GetArray()) {
        m_tiles.push_back(Tile{
            sf::Texture(),
            pair["file"].GetString(),
            json_to_floatrect(pair["bounds"]),
            false,
        });
    }
}


void Background::load_textures() {
    for (auto& tile : m_tiles) {
        std::ignore = tile.texture.loadFromFile(tile.filename);
        tile.loaded = true;
    }

}

void Background::unload_textures() {
    for (auto& tile : m_tiles) {
        tile.texture = sf::Texture();
        tile.loaded = false;
    }
}


void Background::update(const sf::FloatRect& frustum) {
    if (!m_dynamic) { return; }
    for (auto& tile : m_tiles) {
        if (tile.bounds.findIntersection(frustum)) {
            if (!tile.loaded) {
                tile.loaded = true;
                std::ignore = tile.texture.loadFromFile(tile.filename);
                tile.texture.setSmooth(true);
            }
        } else {
            if (tile.loaded) {
                tile.loaded = false;
                tile.texture = sf::Texture();
            }
        }
    }
}

void Background::draw(sf::RenderTarget& target, sf::RenderStates states) const {
    for (auto& tile : m_tiles) {
        if (tile.loaded) {
            auto sprite = sf::Sprite(tile.texture);
            sprite.setPosition(tile.bounds.position);
            target.draw(sprite, states);
        }
    }
}

