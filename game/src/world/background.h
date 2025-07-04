#pragma once
#include <SFML/Graphics.hpp>
#include <rapidjson/document.h>
#include <memory>
#include <atomic>
#include <future>


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

    void load_all();
    void unload_all();

    void update(const sf::FloatRect& frustum);
    void draw(sf::RenderTarget& target, sf::RenderStates states) const;
};


class AsyncBackground : public sf::Drawable {
public:
    enum class LoadState { UNLOADED, LOADING, LOADED, INTERRUPTED };

private:
    struct Tile {
        sf::Texture texture;
        std::string filename;
        sf::FloatRect bounds;
        std::future<sf::Image> progress;
        LoadState load_state = LoadState::UNLOADED;

        Tile() = default;
        Tile(const Tile&) = delete;
        Tile(Tile&& other)
            : texture(std::move(other.texture)), filename(std::move(other.filename)), bounds(other.bounds), progress(std::move(other.progress))
        {}
        Tile(const std::string& _filename, sf::FloatRect _bounds)
            : filename(_filename), bounds(_bounds)
        {}
        Tile(std::string&& _filename, sf::FloatRect _bounds)
            : filename(std::move(_filename)), bounds(_bounds)
        {}
    };

    std::vector<Tile> m_tiles;
    float m_margin = 0.5f;

public:
    AsyncBackground() = default;

    void load_from_json(const rapidjson::Value& value, float margin);
    void load_from_json(const rapidjson::Value& value, float margin, const sf::FloatRect& initial_frustum);

    void reinit_frustum(const sf::FloatRect& frustum);
    void unload_all();
    void update(const sf::FloatRect& frustum);
    void draw(sf::RenderTarget& target, sf::RenderStates states) const;
};

