#include "pch.h"
#include "filters.h"


namespace shmy { namespace filter {

constexpr uint8_t TRANSPARENT_THRESHOLD = 70;

static bool is_border(const sf::Image& img, const sf::Vector2i& pos);
static void fill_outline(sf::Image& result, const sf::Image& tex, const sf::Vector2u& pos, int width);


sf::Image outline(const sf::Image& tex, int width) {
    auto result = sf::Image(tex.getSize(), sf::Color::Transparent);

    for (uint32_t y = 0; y < tex.getSize().y; y++) {
        for (uint32_t x = 0; x < tex.getSize().x; x++) {
            if (tex.getPixel({x, y}).a > TRANSPARENT_THRESHOLD) {
                if (is_border(tex, {(int)x, (int)y})) {
                    fill_outline(result, tex, {x, y}, width);
                }
            }
        }
    }

    return result;
}

sf::Image outline_threaded(const sf::Image& tex, int width) {
    auto result = sf::Image(tex.getSize(), sf::Color::Transparent);
    auto pool = dp::thread_pool(std::thread::hardware_concurrency());

    for (uint32_t i = 0; i < tex.getSize().y; i += 20) {
        pool.enqueue_detach([=, &tex, &result](){
            for (uint32_t y = i; y < i + 20 && y < tex.getSize().y; y++) {
                for (uint32_t x = 0; x < tex.getSize().x; x++) {
                    if (tex.getPixel({x, y}).a > TRANSPARENT_THRESHOLD) {
                        if (is_border(tex, {(int)x, (int)y})) {
                            fill_outline(result, tex, {x, y}, width);
                        }
                    }
                }
            }
        });
    }

    pool.wait_for_tasks();
    return result;
}


sf::Image clickmap(const sf::Image& tex, int width) {
    auto result = sf::Image(tex.getSize(), sf::Color::Transparent);

    for (uint32_t y = 0; y < tex.getSize().y; y++) {
        for (uint32_t x = 0; x < tex.getSize().x; x++) {
            if (tex.getPixel({x, y}).a > TRANSPARENT_THRESHOLD) {
                result.setPixel({x, y}, tex.getPixel({x, y}));
                if (is_border(tex, {(int)x, (int)y})) {
                    fill_outline(result, tex, {x, y}, width);
                }
            }
        }
    }

    return result;
}

sf::Image clickmap_threaded(const sf::Image& tex, int width) {
    auto result = sf::Image(tex.getSize(), sf::Color::Transparent);
    auto pool = dp::thread_pool(std::thread::hardware_concurrency());

    for (uint32_t i = 0; i < tex.getSize().y; i += 20) {
        pool.enqueue_detach([=, &tex, &result](){
            for (uint32_t y = i; y < i + 20 && y < tex.getSize().y; y++) {
                for (uint32_t x = 0; x < tex.getSize().x; x++) {
                    if (tex.getPixel({x, y}).a > TRANSPARENT_THRESHOLD) {
                        result.setPixel({x, y}, tex.getPixel({x, y}));
                        if (is_border(tex, {(int)x, (int)y})) {
                            fill_outline(result, tex, {x, y}, width);
                        }
                    }
                }
            }
        });
    }

    pool.wait_for_tasks();
    return result;
}


static bool is_border(const sf::Image& img, const sf::Vector2i& pos) {
    for (int y = pos.y - 1; y < pos.y + 2; y++) {
        for (int x = pos.x - 1; x < pos.x + 2; x++) {
            if (!(x == pos.x && y == pos.y) && x >= 0 && x < (int)img.getSize().x && y >= 0 && y < (int)img.getSize().y) {
                if (img.getPixel({(uint32_t)x, (uint32_t)y}).a <= TRANSPARENT_THRESHOLD) {
                    return true;
                }
            }
        }
    }

    return false;
}

static void fill_outline(sf::Image& result, const sf::Image& tex, const sf::Vector2u& pos, int width) {
    for (int y = (int)pos.y - width; y < (int)pos.y + width + 1; y++) {
        for (int x = (int)pos.x - width; x < (int)pos.x + width + 1; x++) {
            if (!(y == (int)pos.y && x == (int)pos.x) && y >= 0 && y < (int)tex.getSize().y && x >= 0 && x < (int)tex.getSize().x) {
                const auto diff = sf::Vector2f(sf::Vector2i(pos) - sf::Vector2i(x, y));
                if (diff.lengthSquared() <= (float)(width * width) && tex.getPixel({(uint32_t)x, (uint32_t)y}).a <= TRANSPARENT_THRESHOLD) {
                    const auto l = 1.f - (diff.length() / (float)width);
                    const auto temp = result.getPixel({(uint32_t)x, (uint32_t)y});
                    result.setPixel({(uint32_t)x, (uint32_t)y}, sf::Color(255, 255, 255, std::max(temp.a, (uint8_t)(l * 255.f))));
                }
            }
        }
    }
}


} }
