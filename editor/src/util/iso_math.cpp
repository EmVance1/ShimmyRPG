#include "pch.h"
#include "repr.h"


sf::Transform cart_to_iso(const sf::Vector2f& origin, float scale) {
    auto t = sf::Transform();
    t.translate(origin);
    t.scale({std::sqrt(2.f) / scale, 1.f / scale});
    t.rotate(sf::degrees(45));
    return t;
}

sf::Transform iso_to_cart(const sf::Vector2f& origin, float scale) {
    return cart_to_iso(origin, scale).getInverse();
}


Position map_spaces(const Position& from, Position::Mode to, const sf::Vector2f& origin, float scale) {
    if (from.mode == Position::Mode::World && to == Position::Mode::Iso) {
        return Position{ Position::Mode::Iso, cart_to_iso(origin, scale).transformPoint(from.pos) };
    } else if (from.mode == Position::Mode::Iso && to == Position::Mode::World) {
        return Position{ Position::Mode::World, iso_to_cart(origin, scale).transformPoint(from.pos) };
    } else {
        return from;
    }
}


/*
static sf::IntRect get_bounds(const sf::Image& img) {
    auto bounds = sf::IntRect({0, 0}, {0, 0});

    for (uint32_t x = 0; x < img.getSize().x; x++) {
        for (uint32_t y = 0; y < img.getSize().y; y++) {
            if (img.getPixel({x, y}) != sf::Color::Transparent) {
                bounds.position.x = x;
                goto next1;
            }
        }
    }
next1:

    for (uint32_t y = 0; y < img.getSize().y; y++) {
        for (uint32_t x = 0; x < img.getSize().x; x++) {
            if (img.getPixel({x, y}) != sf::Color::Transparent) {
                bounds.position.y = y;
                goto next2;
            }
        }
    }
next2:

    for (int x = (int)img.getSize().x - 1; x >= 0; x--) {
        for (uint32_t y = 0; y < img.getSize().y; y++) {
            if (img.getPixel({(uint32_t)x, y}) != sf::Color::Transparent) {
                bounds.size.x = x - bounds.position.x;
                goto next3;
            }
        }
    }
next3:

    for (int y = (int)img.getSize().y - 1; y >= 0; y--) {
        for (uint32_t x = 0; x < img.getSize().x; x++) {
            if (img.getPixel({x, (uint32_t)y}) != sf::Color::Transparent) {
                bounds.size.y = y - bounds.position.y;
                goto next4;
            }
        }
    }
next4:

    return bounds;
}

static sf::Image cropped(const sf::Image& img, uint32_t margin) {
    const auto bounds = get_bounds(img);
    auto result = sf::Image(sf::Vector2u(bounds.size) + sf::Vector2u(margin * 2, margin * 2), sf::Color::Transparent);

    for (int y = -1; y < bounds.size.y + 2; y++) {
        for (int x = -1; x < bounds.size.x + 2; x++) {
            const auto p = img.getPixel(sf::Vector2u(x + bounds.position.x, y + bounds.position.y));
            result.setPixel({x + margin, y + margin}, p);
        }
    }

    return result;
}
*/

