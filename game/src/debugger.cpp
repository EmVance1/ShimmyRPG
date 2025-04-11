#include "pch.h"
#include "debugger.h"
#include "world/area.h"


AreaDebugView::AreaDebugView(const std::string& id, float scale)
    : m_pathfinder_texture("res/textures/" + id + "_map.png"), m_pathfinder(m_pathfinder_texture)
{
    m_pathfinder.setScale({scale, scale});
    m_pathfinder.setColor(sf::Color(255, 255, 255, 50));
}


void AreaDebugView::init(const Area* area) {
    p_area = area;

    for (const auto& t : area->m_triggers) {
        auto& shape = m_triggers.emplace_back();
        shape.setPosition(t.bounds.position);
        shape.setSize(t.bounds.size);
        shape.setFillColor(sf::Color::Transparent);
        shape.setOutlineThickness(1);
        shape.setOutlineColor(sf::Color::White);
    }
}

void AreaDebugView::update() {
    m_pathfinder.setTexture(m_pathfinder_texture);
}

void AreaDebugView::handle_event(const sf::Event& event) {
}

void AreaDebugView::render(sf::RenderTarget& target) const {
    target.draw(m_pathfinder, p_area->m_cart_to_iso);
    for (const auto& t : m_triggers) {
        target.draw(t, p_area->m_cart_to_iso);
    }
}

