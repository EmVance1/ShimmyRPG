#include "SFML/Graphics/RenderStates.hpp"
#include "pch.h"
#include "debugger.h"
#include "world/area.h"
#include "objects/trigger.h"
#include "flags.h"
#include "region.h"


void AreaDebugger::init(const Area* area) {
    p_area = area;

    for (const auto tri : area->pathfinder.triangles) {
        const auto b_color = sf::Color((uint8_t)(rand() % 50) + 20, (uint8_t)(rand() % 150) + 105, (uint8_t)(rand() % 150) + 105, 100);
        const auto w_color = sf::Color((uint8_t)(rand() % 55) + 200, (uint8_t)(rand() % 100) + 55, (uint8_t)(rand() % 100) + 55, 100);
        const auto color = tri.weight > 1.f ? w_color : b_color;
        m_pathfinder.push_back(sf::Vertex{ sf::Vector2f(area->pathfinder.vertices[tri.A].x, area->pathfinder.vertices[tri.A].y), color });
        m_pathfinder.push_back(sf::Vertex{ sf::Vector2f(area->pathfinder.vertices[tri.B].x, area->pathfinder.vertices[tri.B].y), color });
        m_pathfinder.push_back(sf::Vertex{ sf::Vector2f(area->pathfinder.vertices[tri.C].x, area->pathfinder.vertices[tri.C].y), color });
    }

    for (const auto& t : area->triggers) {
        auto& shape = m_triggers.emplace_back();
        shape.setOrigin(t.bounds.size * 0.5f);
        shape.setPosition(t.bounds.position + t.bounds.size * 0.5f);
        shape.setRotation(t.bounds.angle);

        shape.setSize(t.bounds.size);
        shape.setFillColor(sf::Color::Transparent);
        shape.setOutlineThickness(1);
        shape.setOutlineColor(sf::Color::Magenta);
    }
    for (const auto& [_, e] : area->entities) {
        if (e.is_character()) {
            auto& shape = m_colliders.emplace_back();
            shape.setPosition(e.get_trigger_collider().position);
            shape.setRadius(e.get_trigger_collider().radius);
            shape.setOrigin({ e.get_trigger_collider().radius, e.get_trigger_collider().radius });
            shape.setFillColor(sf::Color::Transparent);
            shape.setOutlineThickness(1);
            shape.setOutlineColor(sf::Color::Magenta);
        } else {
            auto& shape = m_boundaries.emplace_back(sf::PrimitiveType::Lines);
            shape.append(sf::Vertex(e.get_sorting_boundary().left, sf::Color::Magenta));
            shape.append(sf::Vertex(e.get_sorting_boundary().right, sf::Color::Magenta));
        }
        auto& shape = m_outlines.emplace_back();
        shape.setPosition(e.get_sprite().getPosition() - e.get_sprite().getOrigin());
        shape.setSize(sf::Vector2f(e.get_sprite().getAnimation().getCellSize()));
        shape.setFillColor(sf::Color::Transparent);
        shape.setOutlineThickness(1);
        shape.setOutlineColor(sf::Color::Cyan);
    }
}


void AreaDebugger::update() {
    size_t i = 0;
    size_t j = 0;
    for (const auto& [_, e] : p_area->entities) {
        if (e.is_character()) {
            m_colliders[j].setPosition(e.get_trigger_collider().position);
            j++;
        }
        m_outlines[i].setPosition(e.get_sprite().getPosition() - e.get_sprite().getOrigin());
        i++;
    }

    i = 0;
    for (const auto& t : p_area->triggers) {
        FlagTable::Never = !FlagTable::has_flag(t.once_id);
        if (!t.condition.evaluate() || t.cooldown) {
            m_triggers[i].setOutlineColor(sf::Color(255, 0, 255, 100));
        } else {
            m_triggers[i].setOutlineColor(sf::Color::Magenta);
        }
        i++;
    }

    m_motionguide_line.setCount(p_area->get_player().get_tracker().get_inverse_index() + 1);
}

void AreaDebugger::handle_event(const sf::Event& event) {
    if (event.is<sf::Event::MouseMoved>()) {
        auto path = std::vector<sf::Vector2f>();
        for (const auto& p : p_area->get_player().get_tracker().get_active_path()) { path.push_back({ p.x, p.y }); }
        m_motionguide_line = sfu::LineShape(path);
        m_motionguide_line.setStart(0);
    } else if (event.is<sf::Event::MouseButtonPressed>()) {
        auto path = std::vector<sf::Vector2f>();
        for (const auto& p : p_area->get_player().get_tracker().get_active_path()) { path.push_back({ p.x, p.y }); }
        m_motionguide_line = sfu::LineShape(path);
        m_motionguide_line.setStart(0);
    }
}

void AreaDebugger::render_map(sf::RenderTarget& target) const {
    auto states = sf::RenderStates();
    states.transform = p_area->cart_to_iso;
    target.draw(m_pathfinder.data(), m_pathfinder.size(), sf::PrimitiveType::Triangles, states);
    // if (p_area->get_player().get_tracker().is_moving() || p_area->gamemode == GameMode::Combat) {
    if (p_area->gamemode == GameMode::Normal || p_area->gamemode == GameMode::Combat) {
        target.draw(m_motionguide_line, p_area->cart_to_iso);
    }
}

void AreaDebugger::render(sf::RenderTarget& target) const {
    for (const auto& t : m_triggers) {
        target.draw(t, p_area->cart_to_iso);
    }
    for (const auto& b : m_boundaries) {
        target.draw(b);
    }
    for (const auto& c : m_colliders) {
        target.draw(c, p_area->cart_to_iso);
    }
    for (const auto& o : m_outlines) {
        target.draw(o);
    }
}

