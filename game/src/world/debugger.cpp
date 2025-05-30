#include "pch.h"
#include "debugger.h"
#include "world/area.h"
#include "objects/trigger.h"
#include "flags.h"


AreaDebugView::AreaDebugView(const std::string& id, float scale)
    : m_pathfinder_texture("res/textures/" + id + "_map.png"), m_pathfinder(m_pathfinder_texture)
{
    m_pathfinder.setScale({scale, scale});
    m_pathfinder.setColor(sf::Color(255, 255, 255, 50));
}


void AreaDebugView::init(const Area* area) {
    p_area = area;

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
            shape.append(sf::Vertex(e.get_boundary().left, sf::Color::Magenta));
            shape.append(sf::Vertex(e.get_boundary().right, sf::Color::Magenta));
        }
        auto& shape = m_outlines.emplace_back();
        shape.setPosition(e.get_sprite().getPosition() - e.get_sprite().getOrigin());
        shape.setSize(sf::Vector2f(e.get_sprite().getTexture().getSize()));
        shape.setFillColor(sf::Color::Transparent);
        shape.setOutlineThickness(1);
        shape.setOutlineColor(sf::Color::Cyan);
    }
}


void AreaDebugView::update() {
    m_pathfinder.setTexture(m_pathfinder_texture);
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
        const auto once_id = "once_trig_" + p_area->id + t.id;
        FlagTable::Once = FlagTable::has_flag(once_id);
        if (!t.condition.evaluate() || t.cooldown) {
            m_triggers[i].setOutlineColor(sf::Color(255, 0, 255, 100));
        } else {
            m_triggers[i].setOutlineColor(sf::Color::Magenta);
        }
        i++;
    }

    const auto& player = p_area->get_player();
    m_motionguide_line.set_count(player.get_tracker().get_inverse_index() - 1);
    if (p_area->motionguide_await > 0.05f && p_area->motionguide_await < 10.f) {
        m_motionguide_line = PartialLine::from_path(player.get_tracker().get_active_path());
        m_motionguide_line.setScale({ p_area->scale, p_area->scale });
        m_motionguide_line.set_start(1);
    }
}

void AreaDebugView::handle_event(const sf::Event& event) {
}

void AreaDebugView::render_map(sf::RenderTarget& target) const {
    target.draw(m_pathfinder, p_area->cart_to_iso);
    if (p_area->motionguide_await > 0.05f && p_area->get_player().get_tracker().is_moving()) {
        target.draw(m_motionguide_line, p_area->cart_to_iso);
    }
}

void AreaDebugView::render(sf::RenderTarget& target) const {
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

