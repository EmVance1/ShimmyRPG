#include "pch.h"
#include "background.h"
#include "util/json.h"


FreeListAllocator<sf::Texture> Background::ALLOC(64);

Background::~Background() {
    ALLOC.free_all();
}


void Background::load_from_json(const rapidjson::Value& value) {
    for (const auto& pair : value.GetArray()) {
        m_tiles.push_back(Tile{
            nullptr,
            pair["file"].GetString(),
            json_to_floatrect(pair["bounds"]),
        });
    }
}


void Background::update(const sf::FloatRect& frustum) {
    for (auto& tile : m_tiles) {
        if (tile.bounds.findIntersection(frustum)) {
            if (!tile.texture) {
                tile.texture = ALLOC.alloc();
                auto _ = tile.texture->loadFromFile(tile.filename);
                tile.texture->setSmooth(true);
            }
        } else {
            if (tile.texture) {
                ALLOC.free(tile.texture);
                tile.texture = nullptr;
            }
        }
    }
}

void Background::draw(sf::RenderTarget& target, sf::RenderStates states) const {
    for (auto& tile : m_tiles) {
        if (tile.texture) {
            auto sprite = sf::Sprite(*tile.texture);
            sprite.setPosition(tile.bounds.position);
            target.draw(sprite, states);
        } else {
            std::cout << "nodraw\n";
        }
    }
}

