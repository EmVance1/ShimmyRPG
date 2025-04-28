#include "pch.h"
#include "normal_mode.h"
#include "scripting/speech_graph.h"
#include "world/area.h"
#include "world/region.h"
#include "time/deltatime.h"
#include "sorting.h"
#include "gui/gui.h"


void NormalMode::move_to_action(const std::string& target) {
    const auto& t = p_area->entities.at(target);
    const auto thresh = 15.f;
    if (t.is_character()) {
        p_area->get_player().get_tracker().set_path_world(t.get_tracker().get_position_world());
        p_area->get_player().get_tracker().trim_path_radial_grid(thresh);
    } else {
        const auto abs = t.get_boundary().get_center_of_mass();
        const auto pos = p_area->iso_to_cart.transformPoint(abs);
        p_area->get_player().get_tracker().set_path_world(pos);
    }
    p_area->motionguide_await = 0.f;
}

void NormalMode::speak_action(const std::string& target, const std::string& speech) {
    const auto& t = p_area->entities.at(target);
    const float thresh = 15.f;
    if (sf::Vector2f(p_area->get_player().get_tracker().get_position_grid()
                         - t.get_tracker().get_position_grid()).lengthSquared() < (thresh * thresh * 1.2f)) {
        if (speech.ends_with(".dia")) {
            const auto graph = dialogue_from_file(speech);
            p_area->begin_dialogue(graph);
            p_area->set_mode(GameMode::Cinematic, false);
        } else {
            const auto graph = dialogue_from_line(t.get_script_id(), speech);
            p_area->begin_dialogue(graph);
            p_area->set_mode(GameMode::Cinematic, false);
        }
    } else {
        move_to_action(target);
        p_area->queued = ContextAction{ SpeakAction{ target, speech } };
    }
}



void NormalMode::handle_event(const sf::Event& event) {
    if (p_area->gui.handle_event(event)) {
        if (auto mbp = event.getIf<sf::Event::MouseButtonPressed>()) {
            if (mbp->button == sf::Mouse::Button::Left) {
                if (p_area->gui.has_widget("context_menu")) {
                    p_area->gui.remove_widget("context_menu");
                }
            }
        }
        return;
    }

    if (auto mbp = event.getIf<sf::Event::MouseButtonPressed>()) {
        if (mbp->button == sf::Mouse::Button::Left) {
            const auto mapped = Area::window->mapPixelToCoords(mbp->position, p_area->camera);
            const auto iso = p_area->iso_to_cart.transformPoint(mapped);
            if (!p_area->get_player().is_hovered()) {
                if (p_area->get_player().get_tracker().set_path_world(iso)) {
                    p_area->motionguide_await = 0.f;
                }
            }
            if (p_area->gui.has_widget("context_menu")) {
                p_area->gui.remove_widget("context_menu");
            }
        } else if (mbp->button == sf::Mouse::Button::Right) {
            for (const auto& e : p_area->sorted_entities) {
                if (e->is_hovered()) {
                    if (p_area->gui.has_widget("context_menu")) {
                        p_area->gui.remove_widget("context_menu");
                    }
                    auto ctx_menu = gui::ButtonList::create(
                            gui::Position::topleft(sf::Vector2f(mbp->position)), { 120.f, 30.f }, p_area->p_region->get_style()
                        );
                    for (const auto& action : e->get_actions()) {
                        const auto act = action.to_string();
                        if (strncmp(act, "Move To", 8) == 0) {
                            const auto id = e->get_id();
                            ctx_menu->add_button(act, [&, id](){ move_to_action(id); });
                        } else if (strncmp(act, "Speak", 8) == 0) {
                            const auto id = e->get_id();
                            const auto sp = e->get_dialogue();
                            ctx_menu->add_button(act, [&, id, sp](){ speak_action(id, sp); });
                        } else {
                            ctx_menu->add_button(action.to_string());
                        }
                    }
                    p_area->gui.add_widget("context_menu", ctx_menu);
                }
            }
        }
    } else if (auto mmv = event.getIf<sf::Event::MouseMoved>()) {
        const auto mapped = Area::window->mapPixelToCoords(mmv->position, p_area->camera);
        for (Entity* e : p_area->sorted_entities) {
            e->set_hovered(false);
        }
        if (auto top = top_contains(p_area->sorted_entities, mapped)) {
            top->set_hovered(true);
        }
    } else if (auto scrl = event.getIf<sf::Event::MouseWheelScrolled>()) {
        const auto zoom = (scrl->delta > 0) ? 0.98f : 1.02f;
        const auto begin = p_area->get_player().get_sprite().getPosition();
        p_area->camera.zoom(zoom);
        const auto end = p_area->get_player().get_sprite().getPosition();
        p_area->camera.move(begin - end);
    }
}


void NormalMode::update() {
    for (auto& [_, e] : p_area->entities) {
        e.update_motion(p_area->cart_to_iso);
    }
    for (auto& s : p_area->scripts) {
        s.update();
    }

    if (p_area->queued.has_value() && !p_area->get_player().get_tracker().is_moving()) {
        const auto act = std::get<SpeakAction>(p_area->queued->get_inner());
        speak_action(act.target, act.speech);
        p_area->queued = {};
    }

    p_area->camera.update(Time::deltatime());
    p_area->camera.setTrackingPos(p_area->get_player().get_sprite().getPosition());
    // std::cout << "(" << p_area->camera.getCenter().x << ", " << p_area->camera.getCenter().y << ") " << p_area->id << "\n";

    for (auto& t : p_area->triggers) {
        if (p_area->get_player().get_trigger_collider().intersects(t.bounds)) {
            if (p_area->suppress_triggers) {
                t.cooldown = true;
            }
            if (!t.cooldown) {
                p_area->handle_trigger(t);
            }
            t.cooldown = true;
        } else if (!t.single_use) {
            t.cooldown = false;
        }
    }
    p_area->suppress_triggers = false;

    p_area->gui.update();

}

