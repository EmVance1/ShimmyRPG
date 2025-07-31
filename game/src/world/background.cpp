#include "pch.h"
#include "background.h"
#include "util/json.h"
#include "io/env.h"


AsyncBackground::AsyncBackground(AsyncBackground&& other)
    : m_tiles(std::move(other.m_tiles)), m_pool(), m_margin(other.m_margin)
{}


void AsyncBackground::load_from_json(const rapidjson::Value& value, float margin) {
    m_tiles.reserve(value.GetArray().Size());
    for (const auto& pair : value.GetArray()) {
        m_tiles.emplace_back(
            shmy::env::get() + pair["file"].GetString(),
            json_to_floatrect(pair["bounds"])
        );
    }

    m_margin = margin;
}


void AsyncBackground::unload_all() {
    for (auto& tile : m_tiles) {
        if (tile.load_state == LoadState::LOADED) {
            tile.load_state = LoadState::UNLOADED;
            tile.texture = sf::Texture();
        }
    }
}


void AsyncBackground::update(const sf::FloatRect& frustum) {
    const auto frustum_wide = sf::FloatRect{ frustum.position - frustum.size * m_margin * 0.5f, frustum.size * (1.f + m_margin) };
    auto these_tiles = std::vector<Tile*>();
    int uploads_this_frame = 0;

    for (auto& tile : m_tiles) {
        // LOAD IN CURRENT THREAD
        if (tile.bounds.findIntersection(frustum)) {
            if (tile.load_state == LoadState::UNLOADED) {
                these_tiles.push_back(&tile);
            } else if (tile.load_state == LoadState::LOADING) {
                std::ignore = tile.texture.loadFromImage(tile.progress.get());
                tile.texture.setSmooth(true);
                tile.load_state = LoadState::LOADED;
            }

        // LOAD IN BACKGROUND THREAD
        } else if (tile.bounds.findIntersection(frustum_wide)) {
            if (tile.load_state == LoadState::UNLOADED) {
                tile.load_state = LoadState::LOADING;
                tile.progress = m_pool.enqueue([&](){ return sf::Image(tile.filename); });
            } else if (tile.load_state == LoadState::LOADING && tile.progress.wait_for(std::chrono::seconds(0)) == std::future_status::ready &&
                    uploads_this_frame++ < 2) {
                std::ignore = tile.texture.loadFromImage(tile.progress.get());
                tile.texture.setSmooth(true);
                tile.load_state = LoadState::LOADED;
            }

        // UNLOAD IN CURRENT THREAD
        } else if (tile.load_state == LoadState::LOADING && tile.progress.wait_for(std::chrono::seconds(0)) == std::future_status::ready) {
            tile.progress.get();
            tile.load_state = LoadState::UNLOADED;
        } else if (tile.load_state == LoadState::LOADED) {
            tile.texture = sf::Texture();
            tile.load_state = LoadState::UNLOADED;
        }
    }

    if (these_tiles.size() == 1) {
        std::ignore = these_tiles[0]->texture.loadFromFile(these_tiles[0]->filename);
        these_tiles[0]->load_state = LoadState::LOADED;
    } else if (these_tiles.size() > 1) {
        auto tasks = std::vector<std::future<sf::Image>>();
        tasks.reserve(these_tiles.size());
        for (size_t i = 0; i < these_tiles.size(); i++) {
            tasks.push_back(m_pool.enqueue([&, i](){ return sf::Image(these_tiles[i]->filename); }));
        }
        for (size_t i = 0; i < these_tiles.size(); i++) {
            std::ignore = these_tiles[i]->texture.loadFromImage(tasks[i].get());
            these_tiles[i]->texture.setSmooth(true);
            these_tiles[i]->load_state = LoadState::LOADED;
        }
    }
}

void AsyncBackground::draw(sf::RenderTarget& target, sf::RenderStates states) const {
    for (auto& tile : m_tiles) {
        if (tile.load_state == LoadState::LOADED) {
            auto sprite = sf::Sprite(tile.texture);
            sprite.setPosition(tile.bounds.position);
            target.draw(sprite, states);
        }
    }
}

