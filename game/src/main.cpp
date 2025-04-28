#include "pch.h"
#include "time/deltatime.h"
#include "world/region.h"
#include "world/area.h"
#include "io/load_flags.h"


#ifdef DEBUG
    #define VIDEO_MODE sf::VideoMode({1920, 1080})
    #define SCREEN_MODE sf::Style::Default
#else
    #define VIDEO_MODE sf::VideoMode::getDesktopMode()
    // #define SCREEN_MODE sf::Style::Default
    #define SCREEN_MODE sf::State::Fullscreen
#endif


int main() {
    auto window = sf::RenderWindow(VIDEO_MODE, "Shimmy", SCREEN_MODE);
    // window.setVerticalSyncEnabled(true);
    window.setFramerateLimit(120);
    window.setPosition({0, 0});
    Area::window = &window;

    load_flags();
    auto region = Region();
    region.load_from_folder("res/world/ademmar/");
    region.set_active_area(0);

    const auto font = sf::Font("res/fonts/calibri.ttf");
    auto fps_draw = sf::Text(font, "0", 25);
    fps_draw.setPosition({ 10, 10 });
    fps_draw.setFillColor(sf::Color::White);

    Time::reset();

    while (window.isOpen()) {
        Time::set_frame();

        fps_draw.setString(std::to_string(Time::framerate()) + " FPS");

        while (auto event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) {
                window.close();
            } else if (const auto kyp = event->getIf<sf::Event::KeyPressed>()) {
                if (kyp->code == sf::Keyboard::Key::R && kyp->control) {
                    load_flags();
                    const auto temp = region.get_active_area_index();
                    region.load_from_folder("res/world/ademmar/");
                    region.set_active_area(temp);
                }
            }

            region.get_active_area().handle_event(*event);
        }

        // region.get_active_area().update();
        region.update_all();

        window.clear(sf::Color::Black);

        region.get_active_area().render(window);

        window.setView(window.getDefaultView());

        window.draw(fps_draw);

        window.display();
    }
}

