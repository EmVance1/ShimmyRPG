#include "pch.h"
#include "time/deltatime.h"
#include "world/region.h"
#include "world/area.h"
#include "util/env.h"
#include "util/str.h"
#include "io/load_flags.h"


#ifdef DEBUG
    #define VIDEO_MODE sf::VideoMode({1920, 1080})
    #define SCREEN_MODE sf::Style::Default
#else
    #define VIDEO_MODE sf::VideoMode({ 1920, 1080 }).isValid() ? sf::VideoMode({ 1920, 1080 }) : sf::VideoMode::getDesktopMode()
    // #define SCREEN_MODE sf::Style::Default
    #define SCREEN_MODE sf::State::Fullscreen
#endif


int main(int argc, char** argv) {
    if (argc == 3 && std::string(argv[1]) == "gen" && std::string(argv[2]) == "template") {
        // load project template
    }

    auto window = sf::RenderWindow(VIDEO_MODE, "Shimmy", SCREEN_MODE);
    // window.setVerticalSyncEnabled(true);
    window.setMouseCursorVisible(false);
    window.setPosition({0, 0});
    gui::Widget::WIN_SIZE = window.getSize();

    auto target = sf::RenderTexture();
    auto _ = target.resize(window.getSize());

    auto render_settings = RenderSettings((sf::Vector2i)target.getSize());
    Area::render_settings = &render_settings;

    auto region = Region();
    auto region_folder = std::string("");
    {
        auto startup_file = std::ifstream(".startup");
        auto startup_env = std::string();
        startup_file >> startup_env;
        auto tokens = shmy::str::split(startup_env, ':');
        shmy::env::init(tokens[0]);
        shmy::env::set_pkg(tokens[1]);
        region_folder = tokens[2];
        shmy::flags::load_from_dir(shmy::env::pkg_full() / "flags");
        region.load_from_dir(region_folder, std::atoi(tokens[3].c_str()));
    }

    // auto& pixelate = render_settings.shaders.emplace_back(std::fs::path("res/shaders/pixelate.frag"), sf::Shader::Type::Fragment);
    // pixelate.setUniform("u_resolution", (sf::Vector2f)window.getSize());

    // render_settings.shaders.emplace_back(std::fs::path("res/shaders/poster.frag"), sf::Shader::Type::Fragment);

    // auto& CRT = render_settings.shaders.emplace_back(std::fs::path("res/shaders/CRT.frag"), sf::Shader::Type::Fragment);
    // CRT.setUniform("u_curvature", sf::Vector2f(2.f, 2.f));

    // auto& glitch = render_settings.shaders.emplace_back(std::fs::path("res/shaders/glitch.frag"), sf::Shader::Type::Fragment);
    // glitch.setUniform("u_dist", 3);
    // glitch.setUniform("u_resolution", (sf::Vector2f)window.getSize());

    const auto font = sf::Font(shmy::env::app_dir() / "calibri.ttf");
    auto fps_draw = sf::Text(font, "0", 25);
    fps_draw.setPosition({ 10, 10 });
    fps_draw.setFillColor(sf::Color::White);

    const auto cursor_tex = sf::Texture(shmy::env::pkg_full() / "textures/cursor.png");
    auto cursor = sf::Sprite(cursor_tex);
    cursor.setOrigin({ 4, 3 });

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
                    shmy::flags::load_from_dir(shmy::env::pkg_full() / "flags");
                    const auto temp = region.get_active_area_index();
                    region.load_from_dir(region_folder, temp);
                }
            }

            region.get_active_area().handle_event(*event);
        }

        region.update_all();

        window.clear(sf::Color::Black);

        target.clear(sf::Color::Black);
        region.get_active_area().render_world(target);


        const auto viewcache = target.getView();
        target.setView(target.getDefaultView());
        target.display();
        for (const auto& shader : render_settings.shaders) {
            auto tex = sf::Texture(target.getTexture());
            const auto buff = sf::Sprite(tex);
            target.clear();
            target.draw(buff, &shader);
            target.display();
        }
        target.setView(viewcache);


        auto target_sp = sf::Sprite(target.getTexture());
        target_sp.setTextureRect((sf::IntRect)render_settings.crop);
        target_sp.setPosition((sf::Vector2f)render_settings.crop.position);
        target_sp.setColor(render_settings.overlay);
        // window.setView(sf::View({960, 540}, { 1600, 900 }));
        window.draw(target_sp);
        // window.setView(window.getDefaultView());


        target.clear(sf::Color::Transparent);
        region.get_active_area().render_overlays(target);
        target.setView(target.getDefaultView());
        const auto mouse = sf::Mouse::getPosition(window);
        cursor.setPosition(sf::Vector2f(mouse));
        target.draw(cursor);
        target.draw(fps_draw);

        target.display();
        target_sp = sf::Sprite(target.getTexture());
        window.draw(target_sp);


        window.display();
    }
}

