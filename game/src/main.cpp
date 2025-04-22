#include "pch.h"
#include "time/deltatime.h"
#include "world/region.h"
#include "world/area.h"
#include "flags.h"


#ifdef DEBUG
    #define VIDEO_MODE sf::VideoMode({1920, 1080})
    #define SCREEN_MODE sf::Style::Default
#else
    #define VIDEO_MODE sf::VideoMode::getFullscreenModes()[0]
    #define SCREEN_MODE sf::State::Fullscreen
#endif


void load_flags() {
    FlagTable::set_flag("default", 1);

    auto flag_file = std::ifstream("res/flags/flags.txt");
    auto line = std::string("");
    while (std::getline(flag_file, line)) {
        if (line.empty() || (line[0] == '/' && line[1] == '/')) {
            continue;
        }
        auto stream = std::stringstream(line);
        auto f = std::string("");
        auto v = 0u;
        stream >> f >> v;
        FlagTable::set_flag(f, v);
    }
}


int main() {
    auto window = sf::RenderWindow(sf::VideoMode({1920, 1080}), "Shimmy", SCREEN_MODE);
    // window.setVerticalSyncEnabled(true);
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

