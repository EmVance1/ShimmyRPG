#include "pch.h"
#include "combat_mode.h"
#include "time/deltatime.h"
#include "world/area.h"
#include "sorting.h"
#include "gui/gui.h"


Entity& CombatMode::get_active() {
    return p_area->entities[participants[active_turn].id];
}

const Entity& CombatMode::get_active() const {
    return p_area->entities[participants[active_turn].id];
}


void CombatMode::update_ai() {
    ai_timer -= Time::deltatime();
    if (ai_timer < 0.f) {
        advance_turn = true;
    }
}

bool CombatMode::active_is_playable() const {
    // return get_active().is_playable();
    return participants[active_turn].id == p_area->player_id;
}


void CombatMode::handle_event(const sf::Event& event) {
    p_area->gui.handle_event(event);

    if (const auto kyp = event.getIf<sf::Event::KeyPressed>()) {
        if (kyp->code == sf::Keyboard::Key::E && kyp->control) {
            atk_gui->set_visible(false);
            atk_gui->set_enabled(false);
            p_area->set_mode(GameMode::Normal);
        }
    } else if (auto mbp = event.getIf<sf::Event::MouseButtonPressed>()) {
        if (mbp->button == sf::Mouse::Button::Left) {
            if (active_is_playable()) {
                auto& active = get_active();
                if (!active.is_hovered() && active.get_stats().movement > 0.1f) {
                    const auto mapped = Area::window->mapPixelToCoords(mbp->position, p_area->camera);
                    const auto iso = p_area->iso_to_cart.transformPoint(mapped);

                    active.get_tracker().start();
                    if (active.get_tracker().set_target_position(iso)) {
                        const auto len = active.get_tracker().get_active_path_length();
                        active.get_tracker().clamp_path_walked(active.get_stats().movement);
                        p_area->update_motionguide();
                        active.get_stats().movement -= len;
                    }
                }
            }
            if (p_area->gui.has_widget("context_menu")) {
                p_area->gui.remove_widget("context_menu");
            }
        }
    } else if (auto mmv = event.getIf<sf::Event::MouseMoved>()) {
        const auto mapped = Area::window->mapPixelToCoords(mmv->position, p_area->camera);
        const auto iso = p_area->iso_to_cart.transformPoint(mapped);

        for (Entity* e : p_area->sorted_entities) {
            e->set_hovered(false);
        }
        auto tt = std::dynamic_pointer_cast<gui::TextWidget>(p_area->gui.get_widget("tooltip"));
        tt->set_visible(false);
        if (auto top = top_contains(p_area->sorted_entities, mapped); top && top->is_interactible()) {
            top->set_hovered(true);
            if (top->is_character()) {
                tt->set_position(gui::Position::topleft(sf::Vector2f(mmv->position) - sf::Vector2f(0.f, tt->get_size().y)));
                tt->set_label(p_area->story_name_LUT.at(top->get_script_id()));
                tt->set_visible(true);
            }
        }

        if (active_is_playable()) {
            auto& active = get_active();
            if (!active.is_hovered() && active.get_stats().movement > 0.f && !active.get_tracker().is_moving()) {
                active.get_tracker().start();
                if (active.get_tracker().set_target_position(iso)) {
                    active.get_tracker().clamp_path_walked(active.get_stats().movement);
                    p_area->update_motionguide();
                }
                active.get_tracker().pause();
            }
        }
    } else if (auto scrl = event.getIf<sf::Event::MouseWheelScrolled>()) {
        const auto zoom = (scrl->delta > 0) ? 0.98f : 1.02f;
        const auto begin = p_area->get_player().get_sprite().getPosition();
        p_area->camera.zoom(zoom);
        const auto end = p_area->get_player().get_sprite().getPosition();
        p_area->camera.move(begin - end);
    }
}

void CombatMode::update() {
    for (auto& [_, e] : p_area->entities) {
        if (!e.is_offstage()) {
            e.update(p_area->cart_to_iso);
        }
    }

    if (advance_turn) {
        get_active().get_tracker().stop();
        active_turn = (active_turn + 1) % participants.size();
        get_active().get_stats().reset_turn();
        get_active().get_tracker().start();
        p_area->camera.setTrackingPos(get_active().get_sprite().getPosition());
        advance_turn = false;

        if (active_is_playable()) {
            atk_gui->get_widget<gui::Text>("current_actor")->set_label(p_area->story_name_LUT[get_active().get_script_id()]);
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
        p_area->camera.setTrackingPos(get_active().get_sprite().getPosition());
    }

    // p_area->camera.update(Time::deltatime());
    // p_area->gui.update();
}

