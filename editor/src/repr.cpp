#include "pch.h"
#include "repr.h"
#include "sf_json.h"
#include "texmap.h"


Trigger::Trigger() {
    outline.setFillColor(sf::Color::Transparent);
    outline.setOutlineColor(sf::Color::Cyan);
    outline.setOutlineThickness(1.f);
}

Trigger Trigger::from_json(const nm::json& val) {
    auto result = Trigger();
    result.id = val["id"];
    result.bounds = json_to_floatrect(val["bounds"]);
    result.active_if = val["active_if"];
    // action = json_to_action(val["action"]);
    result.single_use = val["single_use"];

    result.outline.setPosition(result.bounds.position);
    result.outline.setSize(result.bounds.size);
    return result;
}

nm::json Trigger::into_json() const {
    auto result = nm::json();
    result["id"] = id;
    result["bounds"] = floatrect_to_json(bounds);
    result["active_if"] = active_if;
    // result["action"] = action_to_json(action);
    result["single_use"] = single_use;
    return result;
}


float SortBoundary::get_threshold(const sf::Vector2f& point) const {
    if (std::abs(right.x - left.x) < 0.0001f) {
        return point.y;
    }
    if (point.x < left.x || point.x > right.x) {
        return point.y;
    }
    const auto dx = right.x - left.x;
    const auto dy = right.y - left.y;
    const auto m = dy / dx;
    const auto q = left.y - (m * left.x);
    return m * point.x + q;
}

sf::Vector2f SortBoundary::get_center_of_mass() const {
    if (is_point) {
        return left;
    } else {
        return left + (right - left) * 0.5f;
    }
}


Position Position::from_json(const nm::json& val) {
    auto result = Position();
    if (val.contains("grid")) {
        result.mode = Mode::Grid;
        result.pos = sf::Vector2f(json_to_vector2i(val["grid"]));
    } else if (val.contains("world")) {
        result.mode = Mode::World;
        result.pos = json_to_vector2f(val["world"]);
    } else if (val.contains("world_iso")) {
        result.mode = Mode::WorldIso;
        result.pos = json_to_vector2f(val["world_iso"]);
    }
    return result;
}

nm::json Position::into_json() const {
    auto result = nm::json();
    switch (mode) {
    case Mode::Grid:
        result["grid"] = vector2f_to_json(pos);
        break;
    case Mode::World:
        result["world"] = vector2f_to_json(pos);
        break;
    case Mode::WorldIso:
        result["world_iso"] = vector2f_to_json(pos);
        break;
    }
    return result;
}


Entity::Entity()
    : texture("player_placeholder"), sprite(TextureMap::get_texture(texture))
{
    outline.setSize(sf::Vector2f(TextureMap::get_texture(texture).getSize()));
    outline.setFillColor(sf::Color::Transparent);
    outline.setOutlineColor(sf::Color::Magenta);
    outline.setOutlineThickness(1.f);
}

Entity::Entity(const std::string& _texture)
    : texture(_texture), sprite(TextureMap::get_texture(_texture))
{
    outline.setSize(sf::Vector2f(TextureMap::get_texture(texture).getSize()));
    outline.setFillColor(sf::Color::Transparent);
    outline.setOutlineColor(sf::Color::Magenta);
    outline.setOutlineThickness(1.f);

    collider.setRadius(15.f);
    collider.setOrigin({15.f, 15.f});
    collider.setFillColor(sf::Color::Transparent);
    collider.setOutlineColor(sf::Color::Cyan);
    collider.setOutlineThickness(1.f);
}

Entity Entity::from_json(const nm::json& val) {
    auto result = Entity(val["texture"]);
    const auto texsize = result.sprite.getTexture().getSize();
    result.position = Position::from_json(val["position"]);
    if (val.contains("controller")) {
        result.controller.mode = Controller::Mode::Npc;
        result.controller.speed = val["controller"]["speed"];
        result.sprite.setOrigin({ (float)texsize.x * 0.5f, (float)texsize.y * 0.9f });
        result.outline.setOrigin({ (float)texsize.x * 0.5f, (float)texsize.y * 0.9f });
    } else {
        result.collider.setOutlineColor(sf::Color::Transparent);
    }
    if (val.contains("ids")) {
        result.ids.script = val["ids"]["script"];
        result.ids.dialogue = val["ids"]["dialogue"];
    }
    if (val.contains("boundary")) {
        result.boundary.left = json_to_vector2f(val["boundary"][0]);
        result.boundary.right = json_to_vector2f(val["boundary"][1]);
        result.boundary.is_point = false;
    } else {
        result.boundary.left = sf::Vector2f((float)texsize.x * 0.5f, (float)texsize.y * 0.9f);
        result.boundary.is_point = true;
    }
    for (const auto& tag : val["tags"]) {
        result.tags.push_back(tag);
    }
    return result;
}

nm::json Entity::into_json() const {
    auto result = nm::json();
    result["texture"]  = texture;
    result["position"] = position.into_json();
    if (controller.mode == Controller::Mode::Npc) {
        auto _controller = nm::json();
        _controller["speed"] = controller.speed;
        result["controller"] = _controller;
    }
    auto _ids = nm::json();
    _ids["script"]   = ids.script;
    _ids["dialogue"] = ids.dialogue;
    result["ids"] = _ids;
    auto _tags = nm::json();
    for (const auto& tag : tags) {
        _tags.push_back(tag);
    }
    result["tags"] = _tags;
    return result;
}

