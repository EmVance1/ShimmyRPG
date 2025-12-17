#pragma once
#include <SFML/Graphics.hpp>
#include <thread_pool/thread_pool.h>
#include <filesystem>
#include <future>


namespace shmy {

class BackgroundStream : public sf::Drawable {
private:
    enum class LoadState { UNLOADED, LOADING, LOADED };

private:
    struct Tile {
        sf::Texture texture;
        sf::FloatRect bounds;
        std::filesystem::path path;
        std::future<sf::Image> progress;
        LoadState load_state = LoadState::UNLOADED;

        Tile() = default;
        Tile(const Tile&) = delete;
        Tile(Tile&& other) noexcept
            : texture(std::move(other.texture)), bounds(other.bounds), path(std::move(other.path)), progress(std::move(other.progress)),
            load_state(other.load_state)
        {}
        Tile(const std::filesystem::path& _path, sf::FloatRect _bounds)
            : bounds(_bounds), path(_path)
        {}
        Tile(std::filesystem::path&& _path, sf::FloatRect _bounds)
            : bounds(_bounds), path(std::move(_path))
        {}
    };

    std::vector<Tile> m_tiles;
    dp::thread_pool<> m_pool;
    sf::FloatRect m_bounds;
    float m_margin = 0.5f;

public:
    BackgroundStream() = default;
    BackgroundStream(const BackgroundStream&) = delete;
    BackgroundStream(BackgroundStream&& other) noexcept;

    void load_from_folder(const std::filesystem::path& path);
    void set_frustum_margin(float margin);
    const sf::FloatRect& get_bounds() const;

    void unload_all();
    void update(const sf::FloatRect& frustum);
    void update_soft(const sf::FloatRect& frustum);
    void draw(sf::RenderTarget& target, sf::RenderStates states) const;
};

}
