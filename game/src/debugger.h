#pragma once
#include <SFML/Graphics.hpp>
#include <vector>


class Area;

class AreaDebugView {
private:
    sf::Texture m_pathfinder_texture;
    sf::Sprite m_pathfinder;
    std::vector<sf::RectangleShape> m_triggers;
    const Area* p_area;

public:
    AreaDebugView(const std::string& id, float scale);

    void init(const Area* area);

    void update();
    void handle_event(const sf::Event& event);
    void render(sf::RenderTarget& target) const;
};

