#include "pch.h"
#include "graphics/background.h"
#include "util/json.h"
#include "util/env.h"


namespace shmy {

AsyncBackground::AsyncBackground(const AsyncBackground& other)
    : m_margin(other.m_margin)
{
    m_tiles.reserve(other.m_tiles.size());
    for (const auto& tile : other.m_tiles) {
        m_tiles.emplace_back(
            tile.filename,
            tile.bounds
        );
    }
}

AsyncBackground::AsyncBackground(AsyncBackground&& other)
    : m_tiles(std::move(other.m_tiles)), m_margin(other.m_margin)
{}


void AsyncBackground::load_from_json(const rapidjson::Value& value, float margin) {
    m_tiles.reserve(value.GetArray().Size());
    for (const auto& pair : value.GetArray()) {
        m_tiles.emplace_back(
            shmy::env::pkg_full() / pair["file"].GetString(),
            shmy::json::into_floatrect(pair["bounds"])
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
    auto tasks = std::vector<Tile*>();
    int uploads_this_frame = 0;

    for (auto& tile : m_tiles) {
        // LOAD IN CURRENT THREAD
        if (tile.bounds.findIntersection(frustum)) {
            if (tile.load_state == LoadState::UNLOADED) {
                tile.progress = m_pool.enqueue([&](){ return sf::Image(tile.filename); });
                tile.load_state = LoadState::LOADING;
                tasks.push_back(&tile);
            } else if (tile.load_state == LoadState::LOADING) {
                tasks.push_back(&tile);
            }

        // LOAD IN BACKGROUND THREAD
        } else if (tile.bounds.findIntersection(frustum_wide)) {
            if (tile.load_state == LoadState::UNLOADED) {
                tile.progress = m_pool.enqueue([&](){ return sf::Image(tile.filename); });
                tile.load_state = LoadState::LOADING;
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

    for (auto t : tasks) {
        std::ignore = t->texture.loadFromImage(t->progress.get());
        t->texture.setSmooth(true);
        t->load_state = LoadState::LOADED;
    }
}

void AsyncBackground::update_soft(const sf::FloatRect& frustum) {
    int uploads_this_frame = 0;

    for (auto& tile : m_tiles) {
        // LOAD IN BACKGROUND THREAD
        if (tile.bounds.findIntersection(frustum)) {
            if (tile.load_state == LoadState::UNLOADED) {
                tile.progress = m_pool.enqueue([&](){ return sf::Image(tile.filename); });
                tile.load_state = LoadState::LOADING;
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

}
