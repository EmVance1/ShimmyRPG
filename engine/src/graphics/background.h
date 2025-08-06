#pragma once
#include <SFML/Graphics.hpp>
#include <thread_pool/thread_pool.h>
#include <rapidjson/document.h>
#include <filesystem>
#include <future>


namespace shmy {

class AsyncBackground : public sf::Drawable {
public:
    enum class LoadState { UNLOADED, LOADING, LOADED, INTERRUPTED };

private:
    struct Tile {
        sf::Texture texture;
        std::filesystem::path filename;
        sf::FloatRect bounds;
        std::future<sf::Image> progress;
        LoadState load_state = LoadState::UNLOADED;

        Tile() = default;
        Tile(const Tile&) = delete;
        Tile(Tile&& other)
            : texture(std::move(other.texture)), filename(std::move(other.filename)), bounds(other.bounds), progress(std::move(other.progress)),
            load_state(other.load_state)
        {}
        Tile(const std::filesystem::path& _filename, sf::FloatRect _bounds)
            : filename(_filename), bounds(_bounds)
        {}
        Tile(std::filesystem::path&& _filename, sf::FloatRect _bounds)
            : filename(std::move(_filename)), bounds(_bounds)
        {}
    };

    std::vector<Tile> m_tiles;
    dp::thread_pool<> m_pool;
    float m_margin = 0.5f;

public:
    AsyncBackground() = default;
    AsyncBackground(AsyncBackground&& other);

    void load_from_json(const rapidjson::Value& value, float margin);

    void unload_all();
    void update(const sf::FloatRect& frustum);
    void draw(sf::RenderTarget& target, sf::RenderStates states) const;
};

}
