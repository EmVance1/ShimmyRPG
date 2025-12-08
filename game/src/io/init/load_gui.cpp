#include "pch.h"
#include "load_scene.h"
#include "world/area.h"
#include "world/region.h"
#include "gui/gui.h"


void SceneLoader::load_gui() {
    const auto viewport = region->render_settings->viewport;
    auto& gui = region->m_gui;

    {
        auto label = gui::Text::create(gui::Position::topcenter({0.f, 50.f}), sf::Vector2f(300, 35), region->style(), region->get_active_area().name);
        label->set_text_alignment(gui::Position::Alignment::Center);
        label->set_text_padding(0.f);
        gui.add_widget("area_label", label);
    }
    {
        auto gold = gui::Panel::create(gui::Position::topright({-50.f, 30.f}), sf::Vector2f(100, 100), region->style());
        gold->set_background_texture(sf::IntRect{ {150, 0}, {100, 100} });

        auto goldtxt = gui::Text::create(gui::Position::topleft({0.f, 70.f}), sf::Vector2f(100, 30), region->style(), "100");
        goldtxt->set_text_alignment(gui::Position::Alignment::Center);
        goldtxt->set_text_padding(0.f);
        goldtxt->set_sorting_layer(1);

        gold->add_widget("goldtxt", goldtxt);
        gui.add_widget("gold_counter", gold);
    }
    {
        auto tooltip = gui::Text::create(gui::Position::topleft({0.f, 0.f}), sf::Vector2f(100, 35), region->style(), "");
        tooltip->set_text_alignment(gui::Position::Alignment::Center);
        tooltip->set_text_padding(5.f);
        tooltip->set_character_size(25);
        tooltip->set_enabled(false);
        tooltip->set_visible(false);
        tooltip->set_background_texture(sf::IntRect{ {0, 150}, {10, 10} });
        gui.add_widget("tooltip", tooltip);
    }

    {
        auto dialogue_gui = gui::Panel::create(
                gui::Position::bottomcenter({0.f, -250.f}),
                sf::Vector2f((float)viewport.x - 800.f, 90.f),
                region->style());
        dialogue_gui->set_enabled(false);
        dialogue_gui->set_visible(false);
        dialogue_gui->set_background_color(sf::Color::Transparent);
        gui.add_widget("dialogue", dialogue_gui);
        region->cinematic_mode.dia_gui = dialogue_gui;

        auto speaker = gui::Text::create(
                gui::Position::topright({0.f, -50.f}),
                sf::Vector2f(300.f, 50.f),
                region->style(), "");
        speaker->set_sorting_layer(-1);
        speaker->set_text_alignment(gui::Position::Alignment::Center);
        speaker->set_background_texture(sf::IntRect{ {0, 150}, {10, 10} });
        dialogue_gui->add_widget("speaker", speaker);

        auto line = gui::Text::create(
                gui::Position::topright({0.f, 0.f}),
                sf::Vector2f((float)viewport.x - 800.f, 80.f),
                region->style(), "");
        line->set_text_padding(10.f);
        line->set_background_texture(sf::IntRect{ {0, 150}, {10, 10} });
        dialogue_gui->add_widget("lines", line);

        auto choice = gui::ButtonList::create(
                gui::Position::bottomright({0.f, 0.f}),
                sf::Vector2f((float)viewport.x - 800.f, 40.f),
                region->style()
            );
        choice->set_enabled(false);
        choice->set_visible(false);
        dialogue_gui->add_widget("choices", choice);
    }

    {
        auto combat_gui = gui::Panel::create(
                gui::Position::bottomcenter({0.f, -50.f}),
                sf::Vector2f((float)viewport.x - 800.f, 150.f),
                region->style());
        combat_gui->set_enabled(false);
        combat_gui->set_visible(false);
        gui.add_widget("combat", combat_gui);
        region->combat_mode.atk_gui = combat_gui;

        auto actor = gui::Text::create(
                gui::Position::topleft({0.f, -50.f}),
                sf::Vector2f(200, 50),
                region->style(),
                ""
            );
        actor->set_text_alignment(gui::Position::Alignment::Center);
        combat_gui->add_widget("current_actor", actor);

        auto end_turn = gui::Button::create(
                gui::Position::topright({0.f, 0.f}),
                sf::Vector2f(150, 150),
                region->style(),
                ""
            );
        end_turn->set_callback([&](){ region->combat_mode.advance_turn = true; });
        end_turn->set_background_texture(sf::IntRect{ {0, 0}, {150, 150} });
        combat_gui->add_widget("end_turn", end_turn);

        /*
        auto combat_gui = gui::parse("
            <Panel id="combat" bottomcenter="0 50" size="winx - 800 150" hidden>
                <Text id="current_actor" topleft="0 -50" size="200 50" ></Text>
                <Button id="end_turn" topright="0 0" size="150 150" texture="0 0 150 150"/>
            </Panel>
        ");
        combat_gui.select("#end_turn")
            ->set_callback([&](){ combat_mode.advance_turn = true; });
        gui.add_widget("combat", combat_gui);
        */
    }
}

