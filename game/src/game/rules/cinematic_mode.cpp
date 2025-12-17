#include "pch.h"
#include "cinematic_mode.h"
#include "normal_mode.h"
#include "world/game.h"
#include "world/scene.h"
#include "util/env.h"


Scene& CinematicMode::get_scene() {
    return p_game->get_active_scene();
}

void CinematicMode::handle_event(const sf::Event& event) {
    auto& scene = get_scene();

    if (dialogue.get_state() == Dialogue::State::Player) {
        p_game->gui.handle_event(event);
    } else {
        if (auto kyp = event.getIf<sf::Event::KeyPressed>()) {
            if (kyp->code == sf::Keyboard::Key::Space && dialogue.is_playing()) {
                dialogue.advance();
            }
        } else if (auto mbp = event.getIf<sf::Event::MouseButtonPressed>()) {
            if (mbp->button == sf::Mouse::Button::Left && dialogue.is_playing()) {
                dialogue.advance();
            }
        }
    }

    if (dialogue.apply_advance()) {
        if (dialogue.is_playing()) {
            const auto elem = dialogue.get_current_element();
            if (auto line = std::get_if<Dialogue::Line>(&elem)) {
                auto choice_gui = dia_gui->get_widget("choices");
                choice_gui->set_enabled(false);
                choice_gui->set_visible(false);
                auto speaker_gui = dia_gui->get_widget("speaker");
                if (*line->speaker == "Narrator") {
                    std::dynamic_pointer_cast<gui::TextWidget>(speaker_gui)->set_label("Narrator");
                } else {
                    std::dynamic_pointer_cast<gui::TextWidget>(speaker_gui)->set_label(scene.get_entity_by_script_id(*line->speaker).story_id());
                }
                auto line_gui = dia_gui->get_widget("lines");
                line_gui->set_visible(true);
                std::dynamic_pointer_cast<gui::TextWidget>(line_gui)->set_label(*line->line);
            } else if (auto choice = std::get_if<Dialogue::Selection>(&elem)) {
                auto line_gui = dia_gui->get_widget("lines");
                auto choice_gui = std::dynamic_pointer_cast<gui::ButtonList>(dia_gui->get_widget("choices"));
                choice_gui->clear();
                choice_gui->set_enabled(true);
                choice_gui->set_visible(true);
                for (const auto& c : *choice) {
                    auto b = choice_gui->add_button(c.line(), [&, c](){ dialogue.advance(c); });
                    b->set_text_padding(10.f);
                    b->set_background_texture(sf::IntRect{ {0, 150}, {10, 10} });
                }
            }
        } else {
            auto choice_gui = dia_gui->get_widget("choices");
            choice_gui->set_enabled(false);
            choice_gui->set_visible(false);
            scene.set_mode(dialogue.get_init_mode());
            dia_gui->set_enabled(false);
            dia_gui->set_visible(false);
            get_scene().lua_vm.set_paused(false);
            if (const auto fu = dialogue.get_followup()) {
                scene.lua_vm.load_anon(std::string("DispatchEvent") + fu.value());
            }
        }
    }
}

void CinematicMode::update() {
    get_scene().lua_vm.set_paused(dialogue.is_playing());
}

