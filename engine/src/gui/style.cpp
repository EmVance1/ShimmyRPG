#include "pch.h"
#include "gui/style.h"
#include "util/json.h"


namespace gui {

Style::Style(const std::filesystem::path& dir) {
    load_from_dir(dir);
}


bool Style::load_from_dir(const std::filesystem::path& dir) {
    const auto doc = shmy::json::load_from_file(dir / "style.json");

    background_color_1 = shmy::json::into_color(doc["background_color_1"].GetArray());
    background_color_2 = shmy::json::into_color(doc["background_color_2"].GetArray());
    background_color_3 = shmy::json::into_color(doc["background_color_3"].GetArray());
    outline_color_1 = shmy::json::into_color(doc["outline_color_1"].GetArray());
    outline_color_2 = shmy::json::into_color(doc["outline_color_2"].GetArray());
    outline_color_3 = shmy::json::into_color(doc["outline_color_3"].GetArray());
    text_color_1 = shmy::json::into_color(doc["text_color_1"].GetArray());
    text_color_2 = shmy::json::into_color(doc["text_color_2"].GetArray());
    text_color_3 = shmy::json::into_color(doc["text_color_3"].GetArray());
    outline_width = doc["outline_width"].GetFloat();
    default_cell = shmy::json::into_intrect(doc["default_cell"].GetArray());
    const auto _cursor = cursor_texture.loadFromFile(dir / "cursor.png");
    const auto _font = font.openFromFile(dir / doc["font"].GetString());
    if (!doc["textured"].IsTrue()) {
        return _cursor && _font;
    }
    textured = true;
    const auto _atlas = background_texture.loadFromFile(dir / "atlas.png");
    background_texture.setSmooth(true);
    return _cursor && _font && _atlas;
}


sf::Vector2f Position::get_relative_center(const sf::FloatRect& container) const {
    switch (alignment) {
    case Alignment::TopLeft:
        return offset;
    case Alignment::CenterLeft:
        return sf::Vector2f(0.f, container.size.y * 0.5f) + offset;
    case Alignment::BottomLeft:
        return sf::Vector2f(0.f, container.size.y) + offset;
    case Alignment::TopCenter:
        return sf::Vector2f(container.size.x * 0.5f, 0.f) + offset;
    case Alignment::Center:
        return container.size * 0.5f + offset;
    case Alignment::BottomCenter:
        return sf::Vector2f(container.size.x * 0.5f, container.size.y) + offset;
    case Alignment::TopRight:
        return sf::Vector2f(container.size.x, 0.f) + offset;
    case Alignment::CenterRight:
        return sf::Vector2f(container.size.x, container.size.y * 0.5f) + offset;
    case Alignment::BottomRight:
        return container.size + offset;
    }
    return offset;
}

sf::Vector2f Position::get_relative_topleft(const sf::FloatRect& container, const sf::Vector2f& size) const {
    switch (alignment) {
    case Alignment::TopLeft:
        return offset;
    case Alignment::CenterLeft:
        return sf::Vector2f(0.f, container.size.y * 0.5f - size.y * 0.5f) + offset;
    case Alignment::BottomLeft:
        return sf::Vector2f(0.f, container.size.y - size.y) + offset;
    case Alignment::TopCenter:
        return sf::Vector2f(container.size.x * 0.5f - size.x * 0.5f, 0.f) + offset;
    case Alignment::Center:
        return container.size * 0.5f - size * 0.5f + offset;
    case Alignment::BottomCenter:
        return sf::Vector2f(container.size.x * 0.5f - size.x * 0.5f, container.size.y - size.y) + offset;
    case Alignment::TopRight:
        return sf::Vector2f(container.size.x - size.x, 0.f) + offset;
    case Alignment::CenterRight:
        return sf::Vector2f(container.size.x, container.size.y * 0.5f - size.y * 0.5f) + offset;
    case Alignment::BottomRight:
        return container.size - size + offset;
    }
    return offset;
}

}
