#pragma once
#include <SFML/Graphics.hpp>
#include <json.hpp>
#include <vector>
#include <string>

namespace nm = nlohmann;


struct BeginScript { std::string filename; };
struct Popup       { std::string message; };
struct GotoRegion  { std::string filename; };
struct GotoArea    { size_t index; sf::Vector2f spawnpos; bool suppress_triggers; };
using TriggerAction = std::variant<BeginScript, Popup, GotoRegion, GotoArea>;

struct Trigger {
    sf::FloatRect bounds;
    std::string condition;
    TriggerAction action;

    sf::RectangleShape outline;

    Trigger();

    static Trigger from_json(const nm::json& val);
    nm::json into_json() const;
};


struct SortBoundary {
    sf::Vector2f left;
    sf::Vector2f right;
    bool is_point = true;

    float get_threshold(const sf::Vector2f& point) const;
    sf::Vector2f get_center_of_mass() const;
    auto operator<=>(const SortBoundary& b) const {
        if (is_point && b.is_point) {
            return left.y <=> b.left.y;
        } else if (is_point) {
            return left.y <=> b.get_threshold(left);
        } else if (b.is_point) {
            return get_threshold(b.left) <=> b.left.y;
        } else {
            return std::min(left.y, left.y) <=> std::min(b.left.y, b.right.y);
        }
        return std::partial_ordering::equivalent;
    }
};

struct Position {
    enum class Mode { World, Iso };
    Mode mode = Mode::World;
    sf::Vector2f pos;

    static Position from_json(const nm::json& val);
    nm::json into_json() const;
};

struct Controller {
    enum class Mode { None, Npc };
    Mode mode = Mode::None;
    float speed = 0.f;
};

struct Names {
    std::string script;
    std::string dialogue;
};


struct Entity {
    std::string uuid;
    std::string texture;
    Position position;
    Controller controller;
    SortBoundary boundary;
    Names ids;
    std::vector<std::string> tags;

    sf::RectangleShape outline;
    // sf::Sprite sprite;
    sf::CircleShape collider;
    sf::VertexArray draw_boundary;

    Entity();
    Entity(const std::string& tex);

    // sf::FloatRect get_AABB() const { return sprite.getGlobalBounds(); }
    // SortBoundary get_boundary() const {
    //     return {
    //         boundary.left + sprite.getPosition(),
    //         boundary.right + sprite.getPosition(),
    //         boundary.is_point
    //     };
    // }

    static Entity from_json(const nm::json& val);
    nm::json into_json() const;
};

