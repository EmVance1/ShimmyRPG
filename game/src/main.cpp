#include "pch.h"
#include "sfutil/sfutil.h"
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
    window.setVerticalSyncEnabled(true);
    window.setPosition({0, 0});
    Area::window = &window;
    gui::Widget::WIN_SIZE = window.getSize();

    auto target = sfu::PostFx();
    auto _ = target.resize(window.getSize());

    load_flags();
    auto region = Region();
    region.load_from_folder("res/world/ademmar/");
    region.set_active_area(0);

    const auto font = sf::Font("res/fonts/calibri.ttf");
    auto fps_draw = sf::Text(font, "0", 25);
    fps_draw.setPosition({ 10, 10 });
    fps_draw.setFillColor(sf::Color::White);

    const auto cursor_tex = sf::Texture("res/textures/cursor.png");
    auto cursor = sf::Sprite(cursor_tex);
    cursor.setOrigin({ 4, 3 });

    window.setMouseCursorVisible(false);

    Time::reset();

    while (window.isOpen()) {
        Time::set_frame();

        fps_draw.setString(std::to_string(Time::framerate()) + " FPS");

        while (auto event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) {
                window.close();
            } else if (const auto kyp = event->getIf<sf::Event::KeyPressed>()) {
                if (kyp->code == sf::Keyboard::Key::R && kyp->control) {
                    FlagTable::clear();
                    load_flags();
                    const auto temp = region.get_active_area_index();
                    region.load_from_folder("res/world/ademmar/");
                    region.set_active_area(temp);
                }
            }

            region.get_active_area().handle_event(*event);
        }

        region.update_all();

        target.clear(sf::Color::Black);
        region.get_active_area().render(target);
        target.setView(target.getDefaultView());
        const auto mouse = sf::Mouse::getPosition(window);
        cursor.setPosition(sf::Vector2f(mouse));
        target.draw(cursor);
        target.draw(fps_draw);
        target.postFxDisplay();

        window.clear(sf::Color::Black);
        window.draw(sf::Sprite(target.getTexture()));
        window.display();
    }
}

