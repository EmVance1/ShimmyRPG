#include "pch.h"
#include "filters.h"


#define TRANSPARENT_THRESHOLD 70


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
                    result.setPixel({(uint32_t)x, (uint32_t)y}, sf::Color(255, 0, 0, std::max(temp.a, (uint8_t)(l * 255.f))));
                }
            }
        }
    }
}

sf::Image gen_outline(const sf::Image& tex, int width) {
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

sf::Image gen_outline_threaded(const sf::Image& tex, int width) {
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


static float closest_squared(const sf::Image& img, const sf::Vector2i& pos, int radius, const std::function<bool(const sf::Color&)> pred) {
    float min = std::numeric_limits<float>::infinity();

    for (int y = pos.y - radius; y < pos.y + radius + 1; y++) {
        for (int x = pos.x - radius; x < pos.x + radius + 1; x++) {
            if (!(x == pos.x && y == pos.y) && x >= 0 && x < (int)img.getSize().x && y >= 0 && y < (int)img.getSize().y) {
                if (pred(img.getPixel({(uint32_t)x, (uint32_t)y}))) {
                    const auto v = sf::Vector2f((float)x, (float)y) - sf::Vector2f(pos);
                    const auto d = v.lengthSquared();
                    if (d < min) {
                        min = d;
                    }
                }
            }
        }
    }

    return min;
}

sf::Image map_area(const sf::Image& img, int avoid_radius) {
    auto result = sf::Image(img.getSize(), sf::Color::Black);
    const auto thresh = (float)(avoid_radius * avoid_radius);

    for (uint32_t y = 0; y < img.getSize().y; y++) {
        for (uint32_t x = 0; x < img.getSize().x; x++) {
            if (img.getPixel({x, y}).r != 255) {
                const auto c = closest_squared(img, {(int)x, (int)y}, avoid_radius, [](const sf::Color& c){ return c.r == 255; });
                if (c <= thresh) {
                    const auto r = 1.f - std::sqrt(c / thresh);
                    result.setPixel({x, y}, sf::Color((uint8_t)(r * 255.f), 0, 0, 255));
                }
            } else {
                const auto c = closest_squared(img, {(int)x, (int)y}, 2, [](const sf::Color& c){ return c.r != 255; });
                if (c <= 2.f * 2.f) {
                    result.setPixel({x, y}, sf::Color::Red);
                } else {
                    result.setPixel({x, y}, sf::Color::Black);
                }
            }
        }
    }

    return result;
}

sf::Image map_area_threaded(const sf::Image& img, int avoid_radius) {
    auto result = sf::Image(img.getSize(), sf::Color::Black);
    const auto thresh = (float)(avoid_radius * avoid_radius);
    auto pool = dp::thread_pool(std::thread::hardware_concurrency());
    auto futures = std::vector<std::future<void>>(img.getSize().y);

    for (uint32_t y = 0; y < img.getSize().y; y++) {
        pool.enqueue_detach([=, &img, &result]() {
            for (uint32_t x = 0; x < img.getSize().x; x++) {
                if (img.getPixel({x, y}).r != 255) {
                    const auto c = closest_squared(img, {(int)x, (int)y}, avoid_radius, [](const sf::Color& c){ return c.r == 255; });
                    if (c <= thresh) {
                        const auto r = 1.f - std::sqrt(c / thresh);
                        result.setPixel({x, y}, sf::Color((uint8_t)(r * 255.f), 0, 0, 255));
                    }
                } else {
                    const auto c = closest_squared(img, {(int)x, (int)y}, 2, [](const sf::Color& c){ return c.r != 255; });
                    if (c <= 2.f * 2.f) {
                        result.setPixel({x, y}, sf::Color::Red);
                    }
                }
            }
        });
    }

    pool.wait_for_tasks();
    return result;
}



static void fill_clickmap(sf::Image& result, const sf::Image& tex, const sf::Vector2u& pos, int width) {
    for (int y = (int)pos.y - width; y < (int)pos.y + width + 1; y++) {
        for (int x = (int)pos.x - width; x < (int)pos.x + width + 1; x++) {
            if (!(y == (int)pos.y && x == (int)pos.x) && y >= 0 && y < (int)tex.getSize().y && x >= 0 && x < (int)tex.getSize().x) {
                const auto diff = sf::Vector2f(sf::Vector2i(pos) - sf::Vector2i(x, y));
                if (diff.lengthSquared() <= (float)(width * width)) {
                    result.setPixel({(uint32_t)x, (uint32_t)y}, sf::Color(0, 0, 0));
                }
            }
        }
    }
}


sf::Image gen_clickmap(const sf::Image& tex, int width) {
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

sf::Image gen_clickmap_threaded(const sf::Image& tex, int width) {
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

