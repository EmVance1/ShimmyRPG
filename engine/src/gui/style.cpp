#include "pch.h"
#include "gui/style.h"
#include "util/json.h"


namespace gui {

Style::Style(const std::filesystem::path& dir) {
    load_from_dir(dir);
}


bool Style::load_from_dir(const std::filesystem::path& dir) {
    variant.clear();
    const auto doc = shmy::json::load_from_file(dir / "style.json");

    for (const auto& v : doc["variants"].GetArray()) {
        auto& var = variant.emplace_back();
        var.bg_1 = shmy::json::into_color(v["bg_1"].GetArray());
        var.bg_2 = shmy::json::into_color(v["bg_2"].GetArray());
        var.bg_3 = shmy::json::into_color(v["bg_3"].GetArray());
        var.text = shmy::json::into_color(v["text"].GetArray());
    }
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

}
