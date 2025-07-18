#pragma once
#include <SFML/Graphics.hpp>
#include <sfutil/sfutil.h>


namespace gui {

enum class TextureFillMode {
    Repeat,
    Stretch,
};

struct Style {
    sf::Color background_color_1 = sf::Color(255, 255, 255);
    sf::Color background_color_2 = sf::Color(120, 120, 120);
    sf::Color background_color_3 = sf::Color(0, 0, 0);
    sf::Color outline_color_1 = sf::Color(0, 0, 0);
    sf::Color outline_color_2 = sf::Color(120, 120, 120);
    sf::Color outline_color_3 = sf::Color(255, 255, 255);
    sf::Color text_color_1 = sf::Color(0, 0, 0);
    sf::Color text_color_2 = sf::Color(120, 120, 120);
    sf::Color text_color_3 = sf::Color(255, 255, 255);
    sf::Texture background_texture;
    sf::IntRect default_cell = sf::IntRect({ 0, 0 },{ 1, 1 });
    bool textured = false;
    float outline_width = 0.f;
    sf::Font font;

    Style();
    Style(const std::string& folder);

    bool load_from_folder(const std::string& folder);
};

enum class Alignment {
    TopLeft,    CenterLeft,  BottomLeft,
    TopCenter,  Center,      BottomCenter,
    TopRight,   CenterRight, BottomRight,
};

struct Position {
    Alignment alignment;
    sf::Vector2f offset;

    static Position topleft     (const sf::Vector2f& offset) { return Position{ Alignment::TopLeft,      offset }; }
    static Position centerleft  (const sf::Vector2f& offset) { return Position{ Alignment::CenterLeft,   offset }; }
    static Position bottomleft  (const sf::Vector2f& offset) { return Position{ Alignment::BottomLeft,   offset }; }
    static Position topcenter   (const sf::Vector2f& offset) { return Position{ Alignment::TopCenter,    offset }; }
    static Position center      (const sf::Vector2f& offset) { return Position{ Alignment::Center,       offset }; }
    static Position bottomcenter(const sf::Vector2f& offset) { return Position{ Alignment::BottomCenter, offset }; }
    static Position topright    (const sf::Vector2f& offset) { return Position{ Alignment::TopRight,     offset }; }
    static Position centerright (const sf::Vector2f& offset) { return Position{ Alignment::CenterRight,  offset }; }
    static Position bottomright (const sf::Vector2f& offset) { return Position{ Alignment::BottomRight,  offset }; }

    Position moved(const sf::Vector2f& move) const { return Position{ alignment, offset + move }; }

    sf::Vector2f get_relative_center(const sf::FloatRect& container) const;
    sf::Vector2f get_relative_topleft(const sf::FloatRect& container, const sf::Vector2f& size) const;

    sf::Vector2f get_absolute_center(const sf::FloatRect& container) const { return get_relative_center(container) + container.position; }
    sf::Vector2f get_absolute_topleft(const sf::FloatRect& container, const sf::Vector2f& size) const { return get_relative_topleft(container, size) + container.position; }
};

}
