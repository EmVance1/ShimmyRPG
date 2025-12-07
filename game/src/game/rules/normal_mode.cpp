#include "pch.h"
#include "normal_mode.h"
#include "scripting/speech/graph.h"
#include "util/env.h"
#include "gui/gui.h"
#include "world/region.h"
#include "world/area.h"
#include "sorting.h"


Area& NormalMode::get_area() {
    return p_region->get_active_area();
}

void NormalMode::init(Region* _region) {
    p_region = _region;
    gui = &_region->m_gui;
}


void NormalMode::move_to_action(const std::string& target) {
    auto& area = get_area();

    const auto& t = area.entities.at(target);
    const auto thresh = 70.f;
    if (t.is_character()) {
        area.get_player().get_tracker().set_target_position(t.get_tracker().get_position());
        area.get_player().get_tracker().trim_path_radial(thresh);
        area.get_player().get_tracker().start();
    } else {
        const auto abs = t.get_sorting_boundary().get_center_of_mass();
        const auto pos = area.iso_to_cart.transformPoint(abs);
        area.get_player().get_tracker().set_target_position({ pos.x, pos.y });
    }
}

void NormalMode::speak_action(const std::string& target, const std::string& speech) {
    auto& area = get_area();

    const auto& t = area.entities.at(target);
    const float thresh = 70.f;
    const auto dist = area.get_player().get_tracker().get_position() - t.get_tracker().get_position();
    if (sf::Vector2f(dist.x, dist.y).lengthSquared() < (thresh * thresh * 1.1f)) {
        if (speech.ends_with(".shmy")) {
            area.begin_dialogue(shmy::speech::Graph::load_from_file(shmy::env::pkg_full() / speech), speech);
            area.set_mode(GameMode::Dialogue);
        } else {
            area.begin_dialogue(shmy::speech::Graph::create_from_line(t.script_id(), speech), t.script_id());
            area.set_mode(GameMode::Dialogue);
        }
    } else {
        move_to_action(target);
        area.queued = ContextAction{ SpeakAction{ target, speech } };
    }
}



void NormalMode::handle_event(const sf::Event& event) {
    auto& area = get_area();

    if (gui->handle_event(event)) {
        if (auto mbp = event.getIf<sf::Event::MouseButtonPressed>()) {
            if (mbp->button == sf::Mouse::Button::Left) {
                if (gui->has_widget("context_menu")) {
                    gui->remove_widget("context_menu");
                }
            }
        }
        return;
    }

    if (auto mbp = event.getIf<sf::Event::MouseButtonPressed>()) {
        if (mbp->button == sf::Mouse::Button::Left) {
            const auto mapped = area.camera.mapPixelToWorld(mbp->position, p_region->render_settings->viewport);
            const auto iso = area.iso_to_cart.transformPoint(mapped);
            auto& player = area.get_player();
            if (!player.is_hovered()) {
                player.get_tracker().set_target_position({ iso.x, iso.y });
            }
            player.get_tracker().start();
            if (gui->has_widget("context_menu")) {
                gui->remove_widget("context_menu");
            }
        } else if (mbp->button == sf::Mouse::Button::Right) {
            for (const auto& e : area.sorted_entities) {
                if (e->is_hovered() && e->is_interactible()) {
                    if (gui->has_widget("context_menu")) {
                        gui->remove_widget("context_menu");
                    }
                    auto ctx_menu = gui::ButtonList::create(
                        gui::Position::topleft(sf::Vector2f(mbp->position)), { 120.f, 30.f }, area.region->style()
                    );
                    for (const auto& action : e->get_actions()) {
                        const auto act = action.to_string();
                        if (action.index() == ContextAction::ActionID::MoveTo) {
                            const auto& id = e->id();
                            ctx_menu->add_button(act, [&, id](){ move_to_action(id); })
                                ->set_background_texture(sf::IntRect{ {0, 150}, {10, 10} });
                        } else if (action.index() == ContextAction::ActionID::Speak) {
                            const auto& id = e->id();
                            const auto& sp = e->get_dialogue();
                            ctx_menu->add_button(act, [&, id, sp](){ speak_action(id, sp); })
                                ->set_background_texture(sf::IntRect{ {0, 150}, {10, 10} });
                        } else {
                            ctx_menu->add_button(action.to_string())
                                ->set_background_texture(sf::IntRect{ {0, 150}, {10, 10} });
                        }
                    }
                    gui->add_widget("context_menu", ctx_menu);
                }
            }
        }
    } else if (auto mmv = event.getIf<sf::Event::MouseMoved>()) {
        const auto mapped = area.camera.mapPixelToWorld(mmv->position, p_region->render_settings->viewport);
        for (Entity* e : area.sorted_entities) {
            e->set_hovered(false);
        }
        auto tt = std::dynamic_pointer_cast<gui::TextWidget>(gui->get_widget("tooltip"));
        tt->set_visible(false);
        if (auto top = top_contains(area.sorted_entities, mapped); top && top->is_interactible()) {
            top->set_hovered(true);
            if (top->is_character()) {
                tt->set_position(gui::Position::topleft(sf::Vector2f(mmv->position) - sf::Vector2f(0.f, tt->get_size().y)));
                tt->set_label(top->story_id());
                tt->set_visible(true);
            }
        }

        auto& player = area.get_player();
        if (!player.is_hovered() && !player.get_tracker().is_moving()) {
            const auto iso = area.iso_to_cart.transformPoint(mapped);

            player.get_tracker().start();
            if (player.get_tracker().set_target_position({ iso.x, iso.y })) {
                p_region->p_cursor->setColor(sf::Color::White);
            } else {
                p_region->p_cursor->setColor(sf::Color::Red);
            }
            player.get_tracker().pause();
        }

        if (gui->has_widget("context_menu")) {
            gui->remove_widget("context_menu");
        }
    } else if (auto scrl = event.getIf<sf::Event::MouseWheelScrolled>()) {
        const auto begin = area.get_player().get_sprite().getPosition();
        area.camera.zoom(area.camera.getZoom() * ((scrl->delta > 0) ? 0.98f : 1.02f), sfu::Camera::ZoomFunc::Instant);
        const auto end = area.get_player().get_sprite().getPosition();
        area.camera.move(begin - end);
    }
}


void NormalMode::update() {
    auto& area = get_area();

    if (area.queued.has_value() && !area.get_player().get_tracker().is_moving()) {
        const auto act = std::get<SpeakAction>(area.queued->get_inner());
        speak_action(act.target, act.speech);
        area.queued = {};
    }

    area.camera.setTrackingPos(area.get_player().get_sprite().getPosition());

    for (auto& t : area.triggers) {
        if (area.get_player().get_trigger_collider().intersects(t.bounds)) {
            if (!t.cooldown) {
                area.handle_trigger(t);
                t.cooldown = true;
                t.used = true;
            }
        } else {
            t.cooldown = false;
        }
    }
}

