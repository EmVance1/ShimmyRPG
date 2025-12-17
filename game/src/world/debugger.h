#pragma once
#include <SFML/Graphics.hpp>
#include <sfutil/lineshape.h>
#include <vector>


class Game;
struct Scene;

class SceneDebugger {
private:
    std::vector<sf::Vertex> m_pathfinder;
    std::vector<sf::RectangleShape> m_triggers;
    std::vector<sf::RectangleShape> m_outlines;
    std::vector<sf::CircleShape> m_colliders;
    std::vector<sf::VertexArray> m_boundaries;
    sfu::LineShape m_motionguide_line;
    sf::ConvexShape m_origin_marker;
    sf::RectangleShape m_xaxis;
    sf::RectangleShape m_yaxis;

    sf::Font m_font;
    sf::Text m_stats_fps;
    sf::Text m_stats_mouse;

    const Game* p_game;
    const Scene* p_scene;

public:
    SceneDebugger();

    void init(const Scene* scene);

    void update();
    void handle_event(const sf::Event& event);
    void render_map(sf::RenderTarget& target) const;
    void render(sf::RenderTarget& target) const;
    void render_ui(sf::RenderTarget& target) const;
};

