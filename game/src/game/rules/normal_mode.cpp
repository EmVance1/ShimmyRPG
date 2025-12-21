#include "pch.h"
#include "normal_mode.h"
#include "scripting/speech/graph.h"
#include "util/env.h"
#include "gui/gui.h"
#include "world/game.h"
#include "world/scene.h"
#include "sorting.h"


Scene& NormalMode::get_scene() {
    return p_game->get_active_scene();
}

void NormalMode::init(Game* _game) {
    p_game = _game;
    gui = &_game->gui;
}


void NormalMode::move_to_action(const MoveToAction& action) {
    auto& scene = get_scene();

    const auto& t = scene.entities.at(action.target);
    const auto thresh = 1.f;
    if (t.is_character()) {
        scene.get_player().get_tracker().set_target_position(t.get_tracker().get_position());
        scene.get_player().get_tracker().trim_path_radial(thresh);
        scene.get_player().get_tracker().start();
    } else {
        const auto abs = t.get_sorting_boundary().get_center_of_mass();
        const auto pos = scene.screen_to_world.transformPoint(abs);
        scene.get_player().get_tracker().set_target_position({ pos.x, pos.y });
    }
}

void NormalMode::speak_action(const SpeakAction& action) {
    auto& scene = get_scene();

    const float thresh = 1.f;
    const auto& t = scene.entities.at(action.target);
    const auto dist = scene.get_player().get_tracker().get_position() - t.get_world_position(scene.screen_to_world);
    if (sf::Vector2f(dist.x, dist.y).lengthSquared() > (thresh * thresh * 1.1f)) {
        move_to_action(MoveToAction{ action.target });
        scene.queued = ContextAction{ action };
        return;
    }

    if (action.speech.ends_with(".shmy")) {
        scene.begin_dialogue(shmy::speech::Graph::load_from_file(shmy::env::pkg_full() / action.speech), action.speech);
        scene.set_mode(GameMode::Dialogue);
    } else {
        scene.begin_dialogue(shmy::speech::Graph::create_from_line(t.get_script_id(), action.speech), t.get_script_id());
        scene.set_mode(GameMode::Dialogue);
    }
}

void NormalMode::examine_action(const ExamineAction& action) {
    speak_action(SpeakAction{ action.target, action.speech });
}


void NormalMode::handle_event(const sf::Event& event) {
    auto& scene = get_scene();

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
            const auto mapped = scene.camera.mapPixelToWorld(mbp->position, p_game->render_settings.viewport);
            const auto world = scene.screen_to_world.transformPoint(mapped);
            auto& player = scene.get_player();
            if (!player.is_hovered()) {
                player.get_tracker().set_target_position({ world.x, world.y });
            }
            player.get_tracker().start();
            if (gui->has_widget("context_menu")) {
                gui->remove_widget("context_menu");
            }
        } else if (mbp->button == sf::Mouse::Button::Right) {
            for (const auto& e : scene.sorted_entities) {
                if (e->is_hovered() && e->is_interactible()) {
                    if (gui->has_widget("context_menu")) {
                        gui->remove_widget("context_menu");
                    }
                    auto ctx_menu = gui::ButtonList::create(
                        gui::Position::topleft(sf::Vector2f(mbp->position)), { 120.f, 30.f }, scene.game->style
                    );
                    for (const auto& action : e->get_actions()) {
                        const auto act = action.to_string();
                        switch (action.index()) {
                        case ContextAction::ActionID::MoveTo:
                            ctx_menu->add_button(act, [&](){ move_to_action(action.get<MoveToAction>()); })
                                ->set_background_texture(sf::IntRect{ {0, 150}, {10, 10} });
                            break;
                        case ContextAction::ActionID::Speak:
                            ctx_menu->add_button(act, [&](){ speak_action(action.get<SpeakAction>()); })
                                ->set_background_texture(sf::IntRect{ {0, 150}, {10, 10} });
                            break;
                        case ContextAction::ActionID::Examine:
                            ctx_menu->add_button(act, [&](){ examine_action(action.get<ExamineAction>()); })
                                ->set_background_texture(sf::IntRect{ {0, 150}, {10, 10} });
                            break;
                        default:
                            ctx_menu->add_button(action.to_string())
                                ->set_background_texture(sf::IntRect{ {0, 150}, {10, 10} });
                            break;
                        }
                    }
                    gui->add_widget("context_menu", ctx_menu);
                }
            }
        }
    } else if (auto mmv = event.getIf<sf::Event::MouseMoved>()) {
        const auto mapped = scene.camera.mapPixelToWorld(mmv->position, p_game->render_settings.viewport);
        for (Entity* e : scene.sorted_entities) {
            e->set_hovered(false);
        }
        auto tt = std::dynamic_pointer_cast<gui::TextWidget>(gui->get_widget("tooltip"));
        tt->set_visible(false);
        if (auto top = top_contains(scene.sorted_entities, mapped); top && top->is_interactible()) {
            top->set_hovered(true);
            tt->set_position(gui::Position::topleft(sf::Vector2f(mmv->position) - sf::Vector2f(0.f, tt->get_size().y)));
            tt->set_label(top->name());
            tt->set_visible(true);
        }

        auto& player = scene.get_player();
        if (!player.is_hovered() && !player.get_tracker().is_moving()) {
            const auto world = scene.screen_to_world.transformPoint(mapped);

            player.get_tracker().start();
            if (player.get_tracker().set_target_position({ world.x, world.y })) {
                p_game->cursor.setColor(sf::Color::White);
            } else {
                p_game->cursor.setColor(sf::Color::Red);
            }
            player.get_tracker().pause();
        }

        if (gui->has_widget("context_menu")) {
            gui->remove_widget("context_menu");
        }
    } else if (auto scrl = event.getIf<sf::Event::MouseWheelScrolled>()) {
        const auto begin = scene.get_player().get_sprite().getPosition();
        const auto amt = std::clamp(scene.camera.getZoom() * ((scrl->delta > 0) ? 0.98f : 1.02f), 0.1f, 3.f);
        scene.camera.zoom(amt, sfu::Camera::ZoomFunc::Instant);
        const auto end = scene.get_player().get_sprite().getPosition();
        scene.camera.move(begin - end);
    }
}


void NormalMode::update() {
    auto& scene = get_scene();

    if (scene.queued.has_value() && !scene.get_player().get_tracker().is_moving()) {
        speak_action(scene.queued->get<SpeakAction>());
        scene.queued = {};
    }

    scene.camera.setTrackingPos(scene.get_player().get_sprite().getPosition());

    for (auto& t : scene.triggers) {
        if (scene.get_player().get_trigger_collider().intersects(t.bounds)) {
            if (!t.cooldown) {
                scene.handle_trigger(t);
                t.cooldown = true;
                t.used = true;
            }
        } else {
            t.cooldown = false;
        }
    }
}

