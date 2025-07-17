#pragma once
#include <SFML/Graphics.hpp>
#include <sfutil/lineshape.h>
#include <vector>


struct Area;

class AreaDebugView {
private:
    std::vector<sf::Vertex> m_pathfinder;
    std::vector<sf::RectangleShape> m_triggers;
    std::vector<sf::RectangleShape> m_outlines;
    std::vector<sf::CircleShape> m_colliders;
    std::vector<sf::VertexArray> m_boundaries;
    sfu::LineShape m_motionguide_line;
    const Area* p_area;

public:
    void init(const Area* area);

    void update();
    void handle_event(const sf::Event& event);
    void render_map(sf::RenderTarget& target) const;
    void render(sf::RenderTarget& target) const;
};

