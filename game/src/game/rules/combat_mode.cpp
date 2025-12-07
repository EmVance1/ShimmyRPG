#include "pch.h"
#include "combat_mode.h"
#include "util/deltatime.h"
#include "gui/gui.h"
#include "world/area.h"
#include "world/region.h"
#include "sorting.h"


Area& CombatMode::get_area() {
    return p_region->get_active_area();
}


Entity& CombatMode::get_active() {
    return get_area().entities.at(participants[active_turn].id);
}


void CombatMode::update_ai() {
    ai_timer -= Time::deltatime();
    if (ai_timer < 0.f) {
        advance_turn = true;
    }
}

bool CombatMode::active_is_playable() const {
    // return get_active().is_playable();
    return participants[active_turn].id == p_region->get_active_area().player_id;
}


void CombatMode::handle_event(const sf::Event& event) {
    auto& area = get_area();

    p_region->m_gui.handle_event(event);

    if (const auto kyp = event.getIf<sf::Event::KeyPressed>()) {
        if (kyp->code == sf::Keyboard::Key::E && kyp->control) {
            atk_gui->set_visible(false);
            atk_gui->set_enabled(false);
            area.set_mode(GameMode::Normal);
            area.lua_vm.set_paused(false);
        }
    } else if (auto mbp = event.getIf<sf::Event::MouseButtonPressed>()) {
        if (mbp->button == sf::Mouse::Button::Left) {
            if (active_is_playable()) {
                auto& active = get_active();
                if (!active.is_hovered() && active.get_stats().movement > 0.1f) {
                    const auto mapped = area.camera.mapPixelToWorld(mbp->position, p_region->render_settings->viewport);
                    const auto iso = area.iso_to_cart.transformPoint(mapped);
                    active.get_tracker().start();
                    if (active.get_tracker().set_target_position({ iso.x, iso.y })) {
                        const auto len = active.get_tracker().get_active_path_length();
                        active.get_tracker().clamp_path_walked(active.get_stats().movement);
                        active.get_stats().movement -= len;
                    }
                }
            }
            if (p_region->m_gui.has_widget("context_menu")) {
                p_region->m_gui.remove_widget("context_menu");
            }
        }
    } else if (auto mmv = event.getIf<sf::Event::MouseMoved>()) {
        const auto mapped = area.camera.mapPixelToWorld(mmv->position, p_region->render_settings->viewport);
        const auto iso = area.iso_to_cart.transformPoint(mapped);
        for (Entity* e : area.sorted_entities) {
            e->set_hovered(false);
        }
        auto tt = std::dynamic_pointer_cast<gui::TextWidget>(p_region->m_gui.get_widget("tooltip"));
        tt->set_visible(false);
        if (auto top = top_contains(area.sorted_entities, mapped); top && top->is_interactible()) {
            top->set_hovered(true);
            if (top->is_character()) {
                tt->set_position(gui::Position::topleft(sf::Vector2f(mmv->position) - sf::Vector2f(0.f, tt->get_size().y)));
                tt->set_label(top->story_id());
                tt->set_visible(true);
            }
        }

        if (active_is_playable()) {
            auto& active = get_active();
            if (!active.is_hovered() && active.get_stats().movement > 0.f && !active.get_tracker().is_moving()) {
                active.get_tracker().start();
                if (active.get_tracker().set_target_position({ iso.x, iso.y })) {
                    active.get_tracker().clamp_path_walked(active.get_stats().movement);
                    p_region->p_cursor->setColor(sf::Color::White);
                } else {
                    p_region->p_cursor->setColor(sf::Color::Red);
                }
                active.get_tracker().pause();
            }
        }
    } else if (auto scrl = event.getIf<sf::Event::MouseWheelScrolled>()) {
        const auto begin = area.get_player().get_sprite().getPosition();
        area.camera.zoom(area.camera.getZoom() * ((scrl->delta > 0) ? 0.98f : 1.02f), sfu::Camera::ZoomFunc::Instant);
        const auto end = area.get_player().get_sprite().getPosition();
        area.camera.move(begin - end);
    }
}

void CombatMode::update() {
    auto& area = get_area();
    area.lua_vm.set_paused(true);

    if (advance_turn) {
        get_active().get_tracker().stop();
        active_turn = (active_turn + 1) % participants.size();
        get_active().get_stats().reset_turn();
        get_active().get_tracker().start();
        area.camera.setTrackingPos(get_active().get_sprite().getPosition());
        advance_turn = false;

        if (active_is_playable()) {
            atk_gui->get_widget<gui::Text>("current_actor")->set_label(get_active().story_id());
            atk_gui->set_visible(true);
            atk_gui->set_enabled(true);
        } else {
            atk_gui->set_visible(false);
            atk_gui->set_enabled(false);
            ai_timer = 1.f;
        }
    }

    if (!active_is_playable()) {
        update_ai();
    }

    if (get_active().get_tracker().is_moving()) {
        area.camera.setTrackingPos(get_active().get_sprite().getPosition());
    }
}

