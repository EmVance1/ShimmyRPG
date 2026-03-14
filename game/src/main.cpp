#include "pch.h"
#include "audio/lib.h"
#include "util/env.h"
#include "util/deltatime.h"
#include "util/log.h"
#include "config/settings.h"
#include "world/game.h"
#include "world/scene.h"


#define SCREEN_MODE_WINDOWED   sf::Style::Close|sf::Style::Titlebar, sf::State::Windowed
#define SCREEN_MODE_BORDERLESS sf::Style::None, sf::State::Windowed
#define SCREEN_MODE_FULLSCREEN sf::State::Fullscreen
#ifdef SHMY_DEBUG
    #define SCREEN_MODE SCREEN_MODE_WINDOWED
    // #define SCREEN_MODE SCREEN_MODE_BORDERLESS
#else
    #define SCREEN_MODE SCREEN_MODE_BORDERLESS
    // #define SCREEN_MODE SCREEN_MODE_FULLSCREEN
#endif


int main(int argc, char** argv) {
    shmy::audio::create_context();
    Settings::init(".boot");

    auto ctxt = sf::ContextSettings();
    if (Settings::antialiasing == 0) {
        ctxt.antiAliasingLevel = 0;
    } else if (Settings::antialiasing == 1) {
        ctxt.antiAliasingLevel = 4;
    } else if (Settings::antialiasing == 2) {
        ctxt.antiAliasingLevel = 8;
    }
    const auto video_mode = sf::VideoMode{ { Settings::x_resolution, Settings::y_resolution }, Settings::bitsperpixel };
    // const auto video_mode = sf::VideoMode{ { 1280, 900 }, Settings::bitsperpixel };
    auto window = sf::RenderWindow(video_mode, "Shimmy Player - pre-alpha " VANGO_PKG_VERSION, SCREEN_MODE, ctxt);
    if (Settings::enable_vsync) {
        window.setVerticalSyncEnabled(true);
    }
    window.setMouseCursorVisible(false);
    window.setPosition({0, 0});

    auto VIEWPORT = Viewport(window.getSize());
    gui::Widget::viewport = (sf::Vector2f)VIEWPORT.box.size;
    sf::RenderTexture target, fxbuf;
    std::ignore = target.resize((sf::Vector2u)VIEWPORT.box.size);
    std::ignore = fxbuf.resize((sf::Vector2u)VIEWPORT.box.size);
    target.setSmooth(true);
    fxbuf.setSmooth(true);

    auto game = Game(&window, VIEWPORT);
    shmy::env::init(shmy::env::Env::CWD);
    const auto mod_list = shmy::env::pkg_list();
    const auto module = (argc == 1) ? mod_list[0] : std::fs::path(argv[1]);
    if (argc == 1) {
        shmy::core::Logger::info("loading detected module: ", module.string());
    } else if (std::fs::exists(module / ".module")) {
        shmy::core::Logger::info("loading specified module: ", module.string());
    } else {
        shmy::core::Logger::info("specified module: ", module.string(), " does not exist");
        return 1;
    }
    game.reload(module);
    shmy::core::Logger::info("successfully loaded module: ", module.string(), "\n");


    shmy::core::Time::reset();

    while (window.isOpen()) {
        shmy::core::Time::set_frame();

        game.exec_scene_swap();
        game.handle_events();
        if (!window.isOpen()) return 0;
        game.update();

        window.clear();
        game.render(window, &target, &fxbuf);
        window.display();
    }

    shmy::audio::destroy_context();
}

