#pragma once
#include <SFML/Graphics.hpp>
#include <sfutil/sfutil.h>
#include <filesystem>


namespace gui {

struct Style {
    struct Element {
        sf::Color bg_1 = sf::Color(255, 255, 255);
        sf::Color bg_2 = sf::Color(120, 120, 120);
        sf::Color bg_3 = sf::Color(0, 0, 0);
        sf::Color text = sf::Color(0, 0, 0);
    };
    enum class TextureMode {
        Repeat,
        Stretch,
    };

    bool textured = false;
    std::vector<Element> variant{ Element() };
    sf::Texture background_texture;
    sf::Texture cursor_texture;
    sf::Font font;

    Style() = default;
    Style(const std::filesystem::path& dir);
    Style(const Style&) = delete;
    Style(Style&&) = delete;

    bool load_from_dir(const std::filesystem::path& dir);
};

}
