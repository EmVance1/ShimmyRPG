#include "pch.h"
#include "graphics/background.h"


namespace shmy {

static uint32_t read_be32(std::ifstream& f) {
    unsigned char b[4];
    f.read((char*)b, 4);
    return ((uint32_t)b[0] << 24) |
           ((uint32_t)b[1] << 16) |
           ((uint32_t)b[2] << 8)  |
            (uint32_t)b[3];
}

BackgroundStream::BackgroundStream(BackgroundStream&& other) noexcept
    : m_tiles(std::move(other.m_tiles)), m_margin(other.m_margin)
{}


void BackgroundStream::load_from_folder(const std::filesystem::path& path) {
    auto mintl = sf::Vector2f(0, 0);
    auto maxbr = sf::Vector2f(0, 0);

    for (const auto& f : std::filesystem::directory_iterator(path)) {
        if (f.path().extension() != ".png") continue;
        const auto stem = f.path().stem().string();
        const auto split = stem.find('_');
        const auto x_idx = std::atoi(stem.substr(0, split).c_str());
        const auto y_idx = std::atoi(stem.substr(split+1).c_str());
        auto file = std::ifstream(f.path(), std::ios::binary);
        file.seekg(8 + 4 + 4);
        const auto w = read_be32(file);
        const auto h = read_be32(file);
        file.close();
        const auto size = sf::Vector2f((float)w, (float)h);
        const auto tl = sf::Vector2f((float)x_idx * size.x, (float)y_idx * size.y);
        const auto br = tl + (sf::Vector2f)size;
        mintl.x = std::min(tl.x, mintl.x);
        mintl.y = std::min(tl.y, mintl.y);
        maxbr.x = std::max(br.x, maxbr.x);
        maxbr.y = std::max(br.y, maxbr.y);
        const auto bounds = sf::FloatRect(tl, br - tl);
        m_tiles.emplace_back(f.path(), bounds);
    }

    m_bounds = sf::FloatRect(mintl, maxbr - mintl);
}

void BackgroundStream::set_frustum_margin(float margin) {
    m_margin = margin;
}

const sf::FloatRect& BackgroundStream::get_bounds() const {
    return m_bounds;
}


void BackgroundStream::unload_all() {
    for (auto& tile : m_tiles) {
        switch (tile.load_state) {
        case LoadState::LOADED:
            tile.load_state = LoadState::UNLOADED;
            tile.texture = sf::Texture();
            break;
        case LoadState::LOADING:
            tile.load_state = LoadState::UNLOADED;
            tile.progress = std::future<sf::Image>{};
            break;
        default:
            break;
        }
    }
}


void BackgroundStream::update(const sf::FloatRect& frustum) {
    const auto frustum_wide = sf::FloatRect{ frustum.position - frustum.size * m_margin * 0.5f, frustum.size * (1.f + m_margin) };
    auto tasks = std::vector<Tile*>();
    int uploads_this_frame = 0;

    for (auto& tile : m_tiles) {
        // LOAD IN CURRENT THREAD
        if (tile.bounds.findIntersection(frustum)) {
            if (tile.load_state == LoadState::UNLOADED) {
                tile.progress = m_pool.enqueue([&](){ return sf::Image(tile.path); });
                tile.load_state = LoadState::LOADING;
                tasks.push_back(&tile);
            } else if (tile.load_state == LoadState::LOADING) {
                tasks.push_back(&tile);
            }

        // LOAD IN BACKGROUND THREAD
        } else if (tile.bounds.findIntersection(frustum_wide)) {
            if (tile.load_state == LoadState::UNLOADED) {
                tile.progress = m_pool.enqueue([&](){ return sf::Image(tile.path); });
                tile.load_state = LoadState::LOADING;
            } else if (tile.load_state == LoadState::LOADING && tile.progress.wait_for(std::chrono::seconds(0)) == std::future_status::ready &&
                    uploads_this_frame++ < 2) {
                std::ignore = tile.texture.loadFromImage(tile.progress.get());
                tile.texture.setSmooth(true);
                tile.load_state = LoadState::LOADED;
            }

        // UNLOAD IN CURRENT THREAD
        } else if (tile.load_state == LoadState::LOADING) {
            tile.progress = std::future<sf::Image>{};
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

void BackgroundStream::update_soft(const sf::FloatRect& frustum) {
    int uploads_this_frame = 0;

    for (auto& tile : m_tiles) {
        // LOAD IN BACKGROUND THREAD
        if (tile.bounds.findIntersection(frustum)) {
            if (tile.load_state == LoadState::UNLOADED) {
                tile.progress = m_pool.enqueue([&](){ return sf::Image(tile.path); });
                tile.load_state = LoadState::LOADING;
            } else if (tile.load_state == LoadState::LOADING && tile.progress.wait_for(std::chrono::seconds(0)) == std::future_status::ready &&
                    uploads_this_frame++ < 2) {
                std::ignore = tile.texture.loadFromImage(tile.progress.get());
                tile.texture.setSmooth(true);
                tile.load_state = LoadState::LOADED;
            }

        // UNLOAD IN CURRENT THREAD
        } else if (tile.load_state == LoadState::LOADING) {
            tile.progress = std::future<sf::Image>{};
            tile.load_state = LoadState::UNLOADED;
        } else if (tile.load_state == LoadState::LOADED) {
            tile.texture = sf::Texture();
            tile.load_state = LoadState::UNLOADED;
        }
    }
}

void BackgroundStream::draw(sf::RenderTarget& target, sf::RenderStates states) const {
    for (auto& tile : m_tiles) {
        if (tile.load_state == LoadState::LOADED) {
            auto sprite = sf::Sprite(tile.texture);
            sprite.setPosition(tile.bounds.position);
            target.draw(sprite, states);
        }
    }
}

}
