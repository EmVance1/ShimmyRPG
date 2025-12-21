#include "pch.h"
#include "audio/lib.h"
#include "settings.h"
#include "world/game.h"
#include "world/scene.h"
#include "util/env.h"
#include "util/deltatime.h"


#ifdef SHMY_DEBUG
    #define SCREEN_MODE sf::State::Windowed
#else
    #define SCREEN_MODE sf::State::Fullscreen
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
    auto window = sf::RenderWindow(video_mode, "Shimmy Player - pre-alpha " VANGO_PKG_VERSION, SCREEN_MODE, ctxt);
    if (Settings::enable_vsync) {
        window.setVerticalSyncEnabled(true);
    }
    window.setMouseCursorVisible(false);
#ifdef SHMY_DEBUG
    window.setPosition({0, 0});
#endif


    auto render_view = window.getDefaultView();
    /*
    auto render_view = sf::View(sf::FloatRect{ { 0.f, 0.f }, { 1920.f, 1080.f } });
    const float aspectratio = (float)video_mode.size.x / (float)video_mode.size.y;
    if (aspectratio < 16.f / 9.f) {
        const float ratio = ((float)video_mode.size.x / (16.f / 9.f)) / (float)video_mode.size.y;
        render_view.setViewport(sf::FloatRect{ { 0.f, 0.5f * (1.f - ratio) }, { 1.f, ratio } });
    } else if (aspectratio > 16.f / 9.f) {
        const float ratio = ((float)video_mode.size.y / (9.f / 16.f)) / (float)video_mode.size.x;
        render_view.setViewport(sf::FloatRect{ { 0.5f * (1.f - ratio), 0.f }, { ratio, 1.f } });
    }
    */

    gui::Widget::VIEWPORT_SIZE = (sf::Vector2u)render_view.getSize();

    auto target = sf::RenderTexture();
    std::ignore = target.resize(window.getSize());
    target.setSmooth(true);

    auto game = Game(&window);
    const auto mod_list = shmy::env::pkg_list();
    const auto module = (argc == 1) ? mod_list[0] : std::fs::path(argv[1]);
    if (argc == 1) {
        std::cout << "loading detected module: " << module << "\n";
    } else if (std::fs::exists(module / ".module")) {
        std::cout << "loading specified module: " << module << "\n";
    } else {
        std::cout << "specified module: " << module << " does not exist\n";
        return 1;
    }
    game.reload(module);

    // auto& pixelate = render_settings.shaders.emplace_back(shmy::env::pkg_full() / "shaders/pixelate.frag", sf::Shader::Type::Fragment);
    // pixelate.setUniform("u_resolution", (sf::Vector2f)window.getSize());

    // render_settings.shaders.emplace_back(shmy::env::pkg_full() / "shaders/poster.frag", sf::Shader::Type::Fragment);

    // auto& CRT = render_settings.shaders.emplace_back(shmy::env::pkg_full() / "shaders/CRT.frag", sf::Shader::Type::Fragment);
    // CRT.setUniform("u_curvature", sf::Vector2f(5.f, 5.f));

    // auto& glitch = render_settings.shaders.emplace_back(shmy::env::pkg_full() / "shaders/glitch.frag", sf::Shader::Type::Fragment);
    // glitch.setUniform("u_dist", 2);
    // glitch.setUniform("u_resolution", (sf::Vector2f)window.getSize());


    Time::reset();

    while (window.isOpen()) {
        Time::set_frame();

        game.exec_scene_swap();
        game.handle_events();
        game.update();

        window.clear(sf::Color::Black);
        target.clear(sf::Color(10, 10, 10));


        game.get_active_scene().render(target);

        const auto viewcache = target.getView();
        target.setView(render_view);
        target.display();
        for (const auto& shader : game.render_settings.shaders) {
            auto tex = sf::Texture(target.getTexture());
            const auto buff = sf::Sprite(tex);
            target.clear();
            target.draw(buff, &shader);
            target.display();
        }
        target.setView(viewcache);

        auto target_sp = sf::Sprite(target.getTexture());
        target_sp.setTextureRect((sf::IntRect)game.render_settings.crop);
        target_sp.setPosition((sf::Vector2f)game.render_settings.crop.position);
        target_sp.setColor(game.render_settings.overlay);


        window.draw(target_sp);
        game.render(window);

        window.display();
    }
}

