#pragma once
#include <SFML/Graphics.hpp>
#include <sfutil/lineshape.h>
#include <vector>


class Game;
struct Scene;

class Debugger {
private:
    std::unordered_map<std::string, sf::CircleShape> m_colliders;
    std::unordered_map<std::string, sf::VertexArray> m_boundaries;
    std::unordered_map<std::string, sf::RectangleShape> m_outlines;
    std::vector<sf::RectangleShape> m_triggers;
    std::vector<sf::Vertex> m_pathfinder;
    sfu::LineShape m_motionguide_line;
    sf::ConvexShape m_origin_marker;
    sf::RectangleShape m_xaxis;
    sf::RectangleShape m_yaxis;
    bool console_open = false;
    char console[128] = { 0 };
    std::string console_out;

    sf::Vector2i cursor_screen;
    sf::Vector2f cursor_cart;
    sf::Vector2f cursor_world;

    Game* p_game;
    Scene* p_scene;

private:
    void handle_command();

public:
    Debugger() = default;

    void init(Scene* scene);

    void handle_input(const sf::Event& event);
    void update();
    void render_map(sf::RenderTarget& target) const;
    void render(sf::RenderTarget& target) const;
};

