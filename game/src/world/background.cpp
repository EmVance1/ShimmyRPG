#include "pch.h"
#include "background.h"
#include "thread_pool/thread_pool.h"
#include "util/json.h"


Background::Background(bool dynamic) : m_dynamic(dynamic) {
}

void Background::load_from_json(const rapidjson::Value& value) {
    m_tiles.reserve(value.GetArray().Size());
    if (m_dynamic) {
        for (const auto& pair : value.GetArray()) {
            m_tiles.push_back(Tile{
                sf::Texture(),
                std::string("res/textures/") + pair["file"].GetString(),
                json_to_floatrect(pair["bounds"]),
                false,
            });
        }
    } else {
        prep_from_json(value);
        load_all();
    }
}

void Background::prep_from_json(const rapidjson::Value& value) {
    m_tiles.reserve(value.GetArray().Size());
    for (const auto& pair : value.GetArray()) {
        m_tiles.push_back(Tile{
            sf::Texture(),
            std::string("res/textures/") + pair["file"].GetString(),
            json_to_floatrect(pair["bounds"]),
            false,
        });
    }
}


void Background::load_all() {
    switch (m_tiles.size()) {
    case 0: return;
    case 1:
        std::ignore = m_tiles[0].texture.loadFromFile(m_tiles[0].filename);
        m_tiles[0].loaded = true;
        return;
    default:
        auto pool = dp::thread_pool<>();
        for (size_t i = 0; i < m_tiles.size(); i++) {
            pool.enqueue_detach([&, i](){
                m_tiles[i].loaded = true;
                std::ignore = m_tiles[i].texture.loadFromFile(m_tiles[i].filename);
                m_tiles[i].texture.setSmooth(true);
            });
        }
        pool.wait_for_tasks();
    }
}

void Background::unload_all() {
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

