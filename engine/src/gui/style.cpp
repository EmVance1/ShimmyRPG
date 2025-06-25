#include "pch.h"
#include "style.h"
#include "util/str.h"
#include "util/json.h"


namespace gui {

Style::Style() {
    auto _ = font.openFromFile("res/fonts/calibri.ttf");
}

Style::Style(const std::string& filename) {
    load_from_file(filename);
}


bool Style::load_from_file(const std::string& filename) {
    auto src = read_to_string(filename);
    rapidjson::Document doc;
    doc.Parse(src.data());

    background_color_1 = json_to_color(doc["background_color_1"].GetArray());
    background_color_2 = json_to_color(doc["background_color_2"].GetArray());
    background_color_3 = json_to_color(doc["background_color_3"].GetArray());
    outline_color_1 = json_to_color(doc["outline_color_1"].GetArray());
    outline_color_2 = json_to_color(doc["outline_color_2"].GetArray());
    outline_color_3 = json_to_color(doc["outline_color_3"].GetArray());
    text_color_1 = json_to_color(doc["text_color_1"].GetArray());
    text_color_2 = json_to_color(doc["text_color_2"].GetArray());
    text_color_3 = json_to_color(doc["text_color_3"].GetArray());
    outline_width = doc["outline_width"].GetFloat();
    default_cell = json_to_intrect(doc["default_cell"].GetArray());
    if (!doc["background_texture"].IsNull()) {
        const auto b = background_texture.loadFromFile(
                doc["background_texture"].GetString()
            );
        background_texture.setSmooth(true);
        textured = true;
        return b && font.openFromFile(doc["fontfile"].GetString());
    }
    return font.openFromFile(doc["fontfile"].GetString());
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
