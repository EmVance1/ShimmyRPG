#include <vangotest/asserts2.h>
#include <SFML/Graphics.hpp>
#include "time/deltatime.h"
#include "gui/gui.h"


vango_test(test_resize) {
    auto window = sf::RenderWindow(sf::VideoMode({ 800, 800 }), "Shimmy GUI - test resize", sf::Style::Default);
    auto view = sf::View({400, 400}, {800, 800});

    gui::Widget::VIEWPORT_SIZE = sf::Vector2u{ 800, 800 };

    auto gui_style = gui::Style();
    gui_style.load_from_dir("test/res/wooden");
    auto gui_layer = gui::Panel::create(gui::Position::topleft({ 0, 0 }), { 800, 800 }, gui_style);

    while (window.isOpen()) {
        Time::set_frame();

        while (auto event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) {
                window.close();
            } else if (auto rsz = event->getIf<sf::Event::Resized>()) {
                view.setSize((sf::Vector2f)rsz->size);
                view.setCenter((sf::Vector2f)rsz->size * 0.5f);
            }

            gui_layer->handle_event(*event);
        }

        gui_layer->update();

        window.setView(view);
        window.clear(sf::Color::Black);
        window.draw(*gui_layer);
        window.display();
    }
}

