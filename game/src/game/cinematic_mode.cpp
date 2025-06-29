#include "pch.h"
#include "cinematic_mode.h"
#include "normal_mode.h"
#include "world/area.h"
#include "time/deltatime.h"
#include "gui/gui.h"


void CinematicMode::handle_event(const sf::Event& event) {
    if (p_area->dialogue.get_state() == Dialogue::State::Player) {
        p_area->gui.handle_event(event);
    } else {
        if (auto kyp = event.getIf<sf::Event::KeyPressed>()) {
            if (kyp->code == sf::Keyboard::Key::Space && p_area->dialogue.is_playing()) {
                p_area->dialogue.advance();
            }
        } else if (auto mbp = event.getIf<sf::Event::MouseButtonPressed>()) {
            if (mbp->button == sf::Mouse::Button::Left && p_area->dialogue.is_playing()) {
                p_area->dialogue.advance();
            }
        }
    }

    if (p_area->dialogue.apply_advance()) {
        if (p_area->dialogue.is_playing()) {
            const auto elem = p_area->dialogue.get_current_element();
            if (auto line = std::get_if<Dialogue::Line>(&elem)) {
                auto choice_gui = dia_gui->get_widget("choices");
                choice_gui->set_enabled(false);
                choice_gui->set_visible(false);
                auto speaker_gui = dia_gui->get_widget("speaker");
                if (line->speaker == "Narrator") {
                    std::dynamic_pointer_cast<gui::TextWidget>(speaker_gui)->set_label("Narrator");
                } else {
                    std::dynamic_pointer_cast<gui::TextWidget>(speaker_gui)->set_label(p_area->story_name_LUT[line->speaker]);
                }
                auto line_gui = dia_gui->get_widget("lines");
                line_gui->set_visible(true);
                std::dynamic_pointer_cast<gui::TextWidget>(line_gui)->set_label(line->line);
            } else if (auto choice = std::get_if<Dialogue::Selection>(&elem)) {
                auto line_gui = dia_gui->get_widget("lines");
                auto choice_gui = std::dynamic_pointer_cast<gui::ButtonList>(dia_gui->get_widget("choices"));
                choice_gui->clear();
                choice_gui->set_enabled(true);
                choice_gui->set_visible(true);
                for (const auto& c : *choice) {
                    size_t i = c.index;
                    auto b = choice_gui->add_button(c.line, [&, i](){ p_area->dialogue.advance(i); });
                    b->set_text_padding(10.f);
                }
            }
        } else {
            auto choice_gui = dia_gui->get_widget("choices");
            choice_gui->set_enabled(false);
            choice_gui->set_visible(false);
            p_area->set_mode(p_area->dialogue.get_init_mode());
            dia_gui->set_enabled(false);
            dia_gui->set_visible(false);
            if (const auto fu = p_area->dialogue.get_followup()) {
                auto& s = p_area->scripts.emplace_back(*p_area);
                s.load_from_file(fu.value());
            }
        }
    }
}

void CinematicMode::update() {
    for (auto& [_, e] : p_area->entities) {
        e.update_motion(p_area->cart_to_iso);
    }
    if (!p_area->dialogue.is_playing()) {
        for (auto& s : p_area->scripts) {
            s.update();
        }
    }

    p_area->camera.update(Time::deltatime());

    p_area->gui.update();
}

