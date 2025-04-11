#include <SFML/Graphics.hpp>
#include <iostream>


sf::Transform cart_to_isometric(const sf::Vector2f& origin) {
    auto t = sf::Transform();
    t.translate(origin);
    t.scale({std::sqrt(2.f), 1.f});
    t.rotate(sf::degrees(45));
    return t;
}

sf::Transform isometric_to_cart(const sf::Vector2f& origin) {
    auto t = sf::Transform();
    t.rotate(sf::degrees(-45));
    t.scale({1.f / std::sqrt(2.f), 1.f});
    t.translate(-origin);
    return t;
}


template<typename T>
std::ostream& operator<<(std::ostream& stream, const sf::Vector2<T>& v) {
    return stream << v.x << ", " << v.y;
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


void undo_iso(const std::string& filename) {
    const auto img = sf::Texture(filename);
    auto sprite = sf::Sprite(img);

    const auto iso_to_cart = isometric_to_cart(sf::Vector2f((float)img.getSize().x * 0.5f, 0.f));

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

    const auto output = cropped(target.getTexture().copyToImage(), 10);
    _ = output.saveToFile(filename.substr(0, filename.size() - 4) + "_undo.png");
}


int main(int argc, char** argv) {
    if (argc > 1 && strncmp(argv[1], "unwarp", 8) == 0) {
        undo_iso("res/bar.png");
        return 0;
    }

    auto window = sf::RenderWindow(sf::VideoMode({1920, 1080}), "Shimmy Editor", sf::Style::Default);
    const auto tex = sf::Texture("res/bar.png");

    const auto topleft = sf::Vector2f(0.f, 0.f);
    const auto cart_to_iso = cart_to_isometric(topleft);
    const auto iso_to_cart = isometric_to_cart(topleft);

    auto sprite = sf::Sprite(tex);
    sprite.setOrigin({(float)tex.getSize().x * 0.5f, 0});
    sprite.setScale({6.f, 6.f});
    sprite.setPosition(cart_to_iso.transformPoint({500, 100}));

    while (window.isOpen()) {
        while (auto event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) {
                window.close();
            }
        }

        window.clear();

        window.draw(sprite, iso_to_cart);

        window.display();
    }
}

