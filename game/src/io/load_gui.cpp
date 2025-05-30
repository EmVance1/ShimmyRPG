#include "gui/style.h"
#include "pch.h"
#include "world/area.h"
#include "world/region.h"
#include "gui/gui.h"


void Area::load_gui() {
    gui.set_style(p_region->get_style());
    gui.set_size(sf::Vector2f(window->getSize()));
    gui.set_background_color(sf::Color::Transparent);
    {
        auto label = gui::Text::create(gui::Position::topcenter({0.f, 50.f}), sf::Vector2f(300, 35), p_region->get_style(), area_label);
        label->set_text_alignment(gui::Alignment::Center);
        label->set_text_padding(0.f);
        gui.add_widget("area_label", label);
    }
    {
        auto gold = gui::Panel::create(gui::Position::topright({-50.f, 30.f}), sf::Vector2f(100, 100), p_region->get_style());
        // gold->set_background_color(sf::Color::Transparent);
        // gold->set_outline_color(sf::Color::Transparent);

        auto goldtxt = gui::Text::create(gui::Position::topleft({0.f, 70.f}), sf::Vector2f(100, 30), p_region->get_style(), "100");
        goldtxt->set_text_alignment(gui::Alignment::Center);
        goldtxt->set_text_padding(0.f);
        goldtxt->set_sorting_layer(1);

        auto goldimg = gui::Image::create(
                gui::Position::topleft({0.f, 0.f}),
                sf::Vector2f(100, 100),
                p_region->get_style(),
                p_region->m_atlases.at("gold"));

        gold->add_widget("goldtxt", goldtxt);
        gold->add_widget("goldimg", goldimg);
        gui.add_widget("gold_counter", gold);
    }
    {
        auto tooltip = gui::Text::create(gui::Position::topleft({0.f, 0.f}), sf::Vector2f(100, 35), p_region->get_style(), "");
        tooltip->set_text_alignment(gui::Alignment::Center);
        tooltip->set_text_padding(5.f);
        tooltip->set_character_size(25);
        tooltip->set_enabled(false);
        tooltip->set_visible(false);
        gui.add_widget("tooltip", tooltip);
    }
    {
        auto speaker = gui::Text::create(
                gui::Position::bottomcenter({350.f, -350.f}),
                sf::Vector2f(300.f, 50.f),
                p_region->get_style(),
                "");
        speaker->set_visible(false);
        speaker->set_sorting_layer(-1);
        speaker->set_text_alignment(gui::Alignment::Center);
        gui.add_widget("dialogue_speaker", speaker);
    }
    {
        auto line = gui::Text::create(
                gui::Position::bottomcenter({0.f, -250.f}),
                sf::Vector2f((float)window->getSize().x - 800.f, 100.f),
                p_region->get_style(),
                "");
        line->set_enabled(false);
        line->set_visible(false);
        line->set_text_padding(10.f);
        gui.add_widget("dialogue_lines", line);
    }
    {
        auto choice = gui::ButtonList::create(
                gui::Position::bottomcenter({0.f, -250.f}),
                sf::Vector2f((float)window->getSize().x - 800.f, 50.f),
                p_region->get_style()
            );
        choice->set_enabled(false);
        choice->set_visible(false);
        gui.add_widget("dialogue_choices", choice);
    }
}

