#include "pch.h"
#include "background.h"
#include "util/json.h"


void AsyncBackground::load_from_json(const rapidjson::Value& value, float margin) {
    m_tiles.reserve(value.GetArray().Size());
    for (const auto& pair : value.GetArray()) {
        m_tiles.emplace_back(
            std::string("res/textures/") + pair["file"].GetString(),
            json_to_floatrect(pair["bounds"])
        );
    }

    m_margin = margin;
}

void AsyncBackground::load_from_json(const rapidjson::Value& value, float margin, const sf::FloatRect& initial_frustum) {
    load_from_json(value, margin);
    reinit_frustum(initial_frustum);
}

void AsyncBackground::reinit_frustum(const sf::FloatRect& frustum) {
    const auto frustum_wide = sf::FloatRect{ frustum.position - frustum.size * m_margin, frustum.size * (1.f + 2.f * m_margin) };
    auto these_tiles = std::vector<Tile*>();

    for (auto& tile : m_tiles) {
        if (tile.bounds.findIntersection(frustum_wide)) {
            these_tiles.push_back(&tile);
        }
    }

    switch (these_tiles.size()) {
    case 0: return;
    case 1:
        std::ignore = these_tiles[0]->texture.loadFromFile(these_tiles[0]->filename);
        these_tiles[0]->load_state = LoadState::LOADED;
        return;
    default:
        auto pool = dp::thread_pool<>();
        for (size_t i = 0; i < these_tiles.size(); i++) {
            pool.enqueue_detach([&, i](){
                std::ignore = these_tiles[i]->texture.loadFromFile(these_tiles[i]->filename);
                these_tiles[i]->texture.setSmooth(true);
                these_tiles[i]->load_state = LoadState::LOADED;
            });
        }
        pool.wait_for_tasks();
    }
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
    const auto frustum_wide = sf::FloatRect{ frustum.position - frustum.size * m_margin, frustum.size * (1.f + 2.f * m_margin) };
    const auto frustum_narrow = frustum;

    for (auto& tile : m_tiles) {
        // LOAD IN CURRENT THREAD
        if (tile.bounds.findIntersection(frustum_narrow)) {
            if (tile.load_state == LoadState::UNLOADED) {
                std::ignore = tile.texture.loadFromFile(tile.filename);
                tile.texture.setSmooth(true);
                tile.load_state = LoadState::LOADED;
            } else if (tile.load_state == LoadState::LOADING) {
                std::ignore = tile.texture.loadFromImage(tile.progress.get());
                tile.texture.setSmooth(true);
                tile.load_state = LoadState::LOADED;
            }

        // LOAD IN BACKGROUND THREAD
        } else if (tile.bounds.findIntersection(frustum_wide)) {
            if (tile.load_state == LoadState::UNLOADED) {
                tile.load_state = LoadState::LOADING;
                tile.progress = std::async(std::launch::async, [](const std::string* filename){ return sf::Image(*filename); }, &tile.filename);
            } else if (tile.load_state == LoadState::LOADING && tile.progress.wait_for(std::chrono::seconds(0)) == std::future_status::ready) {
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

        // INTERRUPT
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

