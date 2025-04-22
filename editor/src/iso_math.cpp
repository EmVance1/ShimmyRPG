#include "pch.h"
#include "repr.h"


sf::Transform cartesian_to_isometric(const sf::Vector2f& origin) {
    auto t = sf::Transform();
    t.translate(origin);
    t.scale({std::sqrt(2.f), 1.f});
    t.rotate(sf::degrees(45));
    return t;
}

sf::Transform isometric_to_cartesian(const sf::Vector2f& origin) {
    return cartesian_to_isometric(origin).getInverse();
}

sf::Transform grid_to_cartesian(float scale) {
    auto t = sf::Transform();
    t.scale({scale, scale});
    return t;
}

sf::Transform cartesian_to_grid(float scale) {
    return grid_to_cartesian(scale).getInverse();
}

sf::Transform grid_to_isometric(const sf::Vector2f& origin, float scale) {
    auto t = cartesian_to_isometric(origin);
    t *= grid_to_cartesian(scale);
    return t;
}

sf::Transform isometric_to_grid(const sf::Vector2f& origin, float scale) {
    return grid_to_isometric(origin, scale).getInverse();
}


Position map_spaces(const Position& from, Position::Mode to, const sf::Vector2f& origin, float scale) {
    if (from.mode == Position::Mode::Grid && to == Position::Mode::World) {
        return Position{ Position::Mode::World, grid_to_cartesian(scale).transformPoint(from.pos) };
    } else if (from.mode == Position::Mode::Grid && to == Position::Mode::WorldIso) {
        return Position{ Position::Mode::WorldIso, grid_to_isometric(origin, scale).transformPoint(from.pos) };
    } else if (from.mode == Position::Mode::World && to == Position::Mode::Grid) {
        return Position{ Position::Mode::Grid, cartesian_to_grid(scale).transformPoint(from.pos) };
    } else if (from.mode == Position::Mode::World && to == Position::Mode::WorldIso) {
        return Position{ Position::Mode::WorldIso, cartesian_to_isometric(origin).transformPoint(from.pos) };
    } else if (from.mode == Position::Mode::WorldIso && to == Position::Mode::Grid) {
        return Position{ Position::Mode::Grid, isometric_to_grid(origin, scale).transformPoint(from.pos) };
    } else if (from.mode == Position::Mode::WorldIso && to == Position::Mode::World) {
        return Position{ Position::Mode::World, isometric_to_cartesian(origin).transformPoint(from.pos) };
    } else {
        return from;
    }
}


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


sf::Image undo_isometric(const sf::Image& img) {
    const auto tex = sf::Texture(img);
    auto sprite = sf::Sprite(tex);

    const auto iso_to_cart = isometric_to_cartesian(sf::Vector2f((float)img.getSize().x * 0.5f, 0.f));

    const auto a = iso_to_cart.transformPoint(sf::Vector2f(0.f, 0.f));
    const auto b = iso_to_cart.transformPoint(sf::Vector2f((float)img.getSize().x, 0.f));
    const auto c = iso_to_cart.transformPoint(sf::Vector2f(img.getSize()));
    const auto d = iso_to_cart.transformPoint(sf::Vector2f(0.f, (float)img.getSize().y));

    const auto bounds = sf::FloatRect({a.x, b.y}, {std::abs(c.x - a.x), std::abs(d.y - b.y)});
    auto target = sf::RenderTexture();
    auto _ = target.resize(sf::Vector2u(bounds.size));
    sprite.setPosition(-bounds.position);
    target.draw(sprite, iso_to_cart);
    target.display();

    return cropped(target.getTexture().copyToImage(), 10);
}

sf::Texture undo_isometric(const sf::Texture& img) {
    auto sprite = sf::Sprite(img);

    const auto iso_to_cart = isometric_to_cartesian(sf::Vector2f((float)img.getSize().x * 0.5f, 0.f));

    const auto a = iso_to_cart.transformPoint(sf::Vector2f(0.f, 0.f));
    const auto b = iso_to_cart.transformPoint(sf::Vector2f((float)img.getSize().x, 0.f));
    const auto c = iso_to_cart.transformPoint(sf::Vector2f(img.getSize()));
    const auto d = iso_to_cart.transformPoint(sf::Vector2f(0.f, (float)img.getSize().y));

    const auto bounds = sf::FloatRect({a.x, b.y}, {std::abs(c.x - a.x), std::abs(d.y - b.y)});
    auto target = sf::RenderTexture();
    auto _ = target.resize(sf::Vector2u(bounds.size));
    sprite.setPosition(-bounds.position);
    target.draw(sprite, iso_to_cart);
    target.display();

    return sf::Texture(cropped(target.getTexture().copyToImage(), 10));
}


