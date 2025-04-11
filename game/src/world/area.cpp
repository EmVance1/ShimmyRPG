#include "pch.h"
#include "area.h"
#include "util/str.h"
#include "sorting.h"
#include "gui/list.h"
#include "time/deltatime.h"


void Area::update_motionguide() {
    const auto& player = m_entities[m_player_id];
    m_motionguide_line.set_count(player.get_tracker().get_inverse_index());
    if (m_motionguide_await > 0.05f && m_motionguide_await < 10.f) {
        m_motionguide_line = PartialLine::from_path(player.get_tracker().get_active_path(), m_scale, sf::Vector2f(m_scale, m_scale) * 0.5f);
        m_motionguide_square.setPosition(player.get_tracker_target());
        m_motionguide_await = 11.f;
    } else if (m_motionguide_await < 0.05f) {
        m_motionguide_await += Time::deltatime();
    }
}

void Area::handle_trigger(const Trigger& trigger) {
    std::cout << "hit trigger - id: \"" << trigger.id << "\", action: " << trigger.action.index() << "\n";
}



void Area::update() {
    for (auto& [_, e] : m_entities) {
        e.update_motion(m_cart_to_iso);
    }

    m_sorted_entities = sort_sprites(m_entities);

    m_camera.update(Time::deltatime());
    m_camera.setTrackingPos(m_entities[m_player_id].get_sprite().getPosition());

    update_motionguide();

    for (auto& t : m_triggers) {
        if (m_entities[m_player_id].get_trigger_collider().intersects(t.bounds)) {
            if (!t.cooldown) {
                handle_trigger(t);
            }
            t.cooldown = true;
        } else if (!t.single_use) {
            t.cooldown = false;
        }
    }

    m_gui.update();

#ifdef DEBUG
    m_debugger.update();
#endif
}


void Area::handle_event(const sf::Event& event) {
    if (m_gui.handle_event(event)) {
        return;
    }

    if (auto mbp = event.getIf<sf::Event::MouseButtonPressed>()) {
        if (mbp->button == sf::Mouse::Button::Left) {
            const auto mapped = window->mapPixelToCoords(mbp->position, m_camera);
            const auto iso = m_iso_to_cart.transformPoint(mapped);
            if (!m_entities[m_player_id].is_hovered()) {
                if (m_entities[m_player_id].set_tracker_target(iso)) {
                    m_motionguide_await = 0.f;
                }
            }
            if (m_gui.has_widget("context_menu")) {
                m_gui.remove_widget("context_menu");
            }
        } else if (mbp->button == sf::Mouse::Button::Right) {
            for (const auto& e : m_sorted_entities) {
                if (e->is_hovered()) {
                    m_gui.add_widget("context_menu", gui::ButtonList::create(
                            gui::Position::topleft(sf::Vector2f(mbp->position)),
                            { 100.f, 30.f },
                            m_guistyle,
                            { "Move to", "Attack" }
                        )
                    );
                }
            }
        }
    } else if (auto mmv = event.getIf<sf::Event::MouseMoved>()) {
        const auto mapped = window->mapPixelToCoords(mmv->position, m_camera);
        for (Entity* e : m_sorted_entities) {
            e->set_hovered(false);
        }
        auto top = top_contains(m_sorted_entities, mapped);
        if (top) {
            top->set_hovered(true);
        }
    } else if (auto scrl = event.getIf<sf::Event::MouseWheelScrolled>()) {
        const auto zoom = (scrl->delta > 0) ? 0.98f : 1.02f;
        const auto begin = window->mapPixelToCoords(scrl->position, m_camera);
        m_camera.zoom(zoom);
        const auto end = window->mapPixelToCoords(scrl->position, m_camera);
        m_camera.move(begin - end);
    }


#ifdef DEBUG
    m_debugger.handle_event(event);
#endif
}


void Area::render(sf::RenderTarget& target) {
    target.setView(m_camera);

    target.draw(m_background);

    if (m_motionguide_await > 0.05f && m_entities[m_player_id].is_moving()) {
        target.draw(m_motionguide_line, m_cart_to_iso);
        target.draw(m_motionguide_square, m_cart_to_iso);
    }

#ifdef DEBUG
    m_debugger.render(target);
#endif

    for (const auto& e : m_sorted_entities) {
        target.draw(e->get_sprite());
    }

    target.setView(target.getDefaultView());
    target.draw(m_gui);
}

