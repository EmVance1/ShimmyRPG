#include "pch.h"
#include "combat_mode.h"
#include "util/deltatime.h"
#include "gui/gui.h"
#include "world/game.h"
#include "world/scene.h"
#include "sorting.h"


Scene& CombatMode::get_scene() {
    return p_game->get_active_scene();
}


Entity& CombatMode::get_active() {
    return get_scene().entities.at(participants[active_turn].id);
}


void CombatMode::update_ai() {
    ai_timer -= Time::deltatime();
    if (ai_timer < 0.f) {
        advance_turn = true;
    }
}

bool CombatMode::active_is_playable() const {
    // return get_active().is_playable();
    return participants[active_turn].id == p_game->get_active_scene().player_id;
}


void CombatMode::handle_event(const sf::Event& event) {
    auto& scene = get_scene();

    p_game->gui.handle_event(event);

    if (const auto kyp = event.getIf<sf::Event::KeyPressed>()) {
        if (kyp->code == sf::Keyboard::Key::E && kyp->control) {
            atk_gui->set_visible(false);
            atk_gui->set_enabled(false);
            scene.set_mode(GameMode::Normal);
            scene.lua_vm.set_paused(false);
        }
    } else if (auto mbp = event.getIf<sf::Event::MouseButtonPressed>()) {
        if (mbp->button == sf::Mouse::Button::Left) {
            if (active_is_playable()) {
                auto& active = get_active();
                if (!active.is_hovered() && active.get_stats().movement > 0.1f) {
                    const auto mapped = scene.camera.mapPixelToWorld(mbp->position, p_game->render_settings.viewport);
                    const auto world = scene.screen_to_world.transformPoint(mapped);
                    active.get_tracker().start();
                    if (active.get_tracker().set_target_position({ world.x, world.y })) {
                        const auto len = active.get_tracker().get_active_path_length();
                        active.get_tracker().clamp_path_walked(active.get_stats().movement);
                        active.get_stats().movement -= len;
                    }
                }
            }
            if (p_game->gui.has_widget("context_menu")) {
                p_game->gui.remove_widget("context_menu");
            }
        }
    } else if (auto mmv = event.getIf<sf::Event::MouseMoved>()) {
        const auto mapped = scene.camera.mapPixelToWorld(mmv->position, p_game->render_settings.viewport);
        const auto world = scene.screen_to_world.transformPoint(mapped);
        for (Entity* e : scene.sorted_entities) {
            e->set_hovered(false);
        }
        auto tt = std::dynamic_pointer_cast<gui::TextWidget>(p_game->gui.get_widget("tooltip"));
        tt->set_visible(false);
        if (auto top = top_contains(scene.sorted_entities, mapped); top && top->is_interactible()) {
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
                if (active.get_tracker().set_target_position({ world.x, world.y })) {
                    active.get_tracker().clamp_path_walked(active.get_stats().movement);
                    p_game->cursor.setColor(sf::Color::White);
                } else {
                    p_game->cursor.setColor(sf::Color::Red);
                }
                active.get_tracker().pause();
            }
        }
    } else if (auto scrl = event.getIf<sf::Event::MouseWheelScrolled>()) {
        const auto begin = scene.get_player().get_sprite().getPosition();
        scene.camera.zoom(scene.camera.getZoom() * ((scrl->delta > 0) ? 0.98f : 1.02f), sfu::Camera::ZoomFunc::Instant);
        const auto end = scene.get_player().get_sprite().getPosition();
        scene.camera.move(begin - end);
    }
}

void CombatMode::update() {
    auto& scene = get_scene();
    scene.lua_vm.set_paused(true);

    if (advance_turn) {
        get_active().get_tracker().stop();
        active_turn = (active_turn + 1) % participants.size();
        get_active().get_stats().reset_turn();
        get_active().get_tracker().start();
        scene.camera.setTrackingPos(get_active().get_sprite().getPosition());
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
        scene.camera.setTrackingPos(get_active().get_sprite().getPosition());
    }
}

