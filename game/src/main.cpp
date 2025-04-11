#include "pch.h"
#include "time/deltatime.h"
#include "world/region.h"
#include "world/area.h"


#ifdef DEBUG
    #define VIDEO_MODE sf::VideoMode({1920, 1080})
    #define SCREEN_MODE sf::Style::Default
#else
    #define VIDEO_MODE sf::VideoMode::getFullscreenModes()[0]
    #define SCREEN_MODE sf::State::Fullscreen
#endif


int main() {
    auto window = sf::RenderWindow(sf::VideoMode({1920, 1080}), "Shimmy", SCREEN_MODE);
    // window.setVerticalSyncEnabled(true);
    window.setPosition({0, 0});
    Area::window = &window;

    auto region = Region();
    region.load_from_file("res/areas/bar.json");

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
                    region.load_from_file("res/areas/bar.json");
                }
            }

            region.active_area().handle_event(*event);
        }

        region.active_area().update();

        window.clear(sf::Color::Black);

        region.active_area().render(window);

        window.setView(window.getDefaultView());

        window.draw(fps_draw);

        window.display();
    }
}

