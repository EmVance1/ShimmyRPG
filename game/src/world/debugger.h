#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include "graphics/partialline.h"


struct Area;

class AreaDebugView {
private:
    sf::Texture m_pathfinder_texture;
    sf::Sprite m_pathfinder;
    std::vector<sf::RectangleShape> m_triggers;
    std::vector<sf::RectangleShape> m_outlines;
    std::vector<sf::CircleShape> m_colliders;
    std::vector<sf::VertexArray> m_boundaries;
    PartialLine m_motionguide_line;
    const Area* p_area;

public:
    AreaDebugView(const std::string& id, float scale);

    void init(const Area* area);

    void update();
    void handle_event(const sf::Event& event);
    void render_map(sf::RenderTarget& target) const;
    void render(sf::RenderTarget& target) const;
};

