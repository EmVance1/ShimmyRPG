#include "pch.h"
#include "game.h"
#include "core/split.h"
#include "util/env.h"
#include "util/json.h"
#include "util/deltatime.h"
#include "assets/manager.h"
#include "assets/flags.h"
#include "assets/init/load_scene.h"


Game::Game(sf::RenderWindow* _window)
    : window(_window),
    render_settings((sf::Vector2i)window->getSize()),
    gui(gui::Position::topleft({0, 0}), sf::Vector2f(render_settings.viewport), gui::Style()),
    cursor(style.cursor_texture)
{
    cursor.setOrigin({ 4, 3 });
}

void Game::reload(const std::fs::path& _module, int start_area) {
    scenes.clear();
    FlagTable::reset();
    AssetManager::reset();

    module = _module;
    auto mod_file = std::ifstream(module / ".module");
    auto mod_init = std::string();
    mod_file >> mod_init;
    auto tokens = shmy::core::split(mod_init, ':');
    region = tokens[0];
    shmy::env::set_pkg(module);
    active_scene = (start_area == -1) ? std::atoi(tokens[1].c_str()) : start_area;

    try {
        FlagTable::init(shmy::env::pkg_full() / "flags");
        AssetManager::init(shmy::env::pkg_full() / "bundles");
    } catch (const std::exception& e) {
        std::cerr << "load module error - " << e.what() << "\n";
        exit(1);
    }

    const auto doc = shmy::json::load_from_file(shmy::env::pkg_full() / region / "region.json");

#ifdef _WIN32
    if (!style.load_from_dir(shmy::env::pkg_full() / "gui" / doc["gui_style"].GetString())) {
        std::cerr << "error loading GUI style\n";
        exit(1);
    }
#else
    std::cout << "1 - scenes: " << m_scenes.size() << ", active: " << m_active_scene << ", passed in: " << initial_scene << "\n";
    // style.load_from_dir(shmy::env::pkg_full() / "gui" / doc["gui_style"].GetString());
    std::cout << "2 - scenes: " << m_scenes.size() << ", active: " << m_active_scene << ", passed in: " << initial_scene << "\n";
#endif

    gui.set_style(style);
    gui.set_size((sf::Vector2f)render_settings.viewport);
    gui.set_background_color(sf::Color::Transparent);
    cursor.setTexture(style.cursor_texture, true);

    const auto& jscenes = doc["areas"].GetArray();
    scenes.reserve(jscenes.Size());
    auto loader = SceneLoader(this, doc["props"]);

    int i = 0;
    for (const auto& scene : jscenes) {
#ifdef SHMY_DEBUG
        try {
            scenes.emplace_back(loader, region, scene.GetString());
        } catch (const std::exception& e) {
            std::cerr << "load module error - scene '" << scene.GetString() << "': " << e.what() << "\n";
            exit(1);
        }
#else
        scenes.emplace_back(loader, region, scene.GetString());
#endif
        const auto FADETIME = 400;
        if (i == active_scene) {
            scenes[(size_t)i].set_sleeping(false);
            for (const auto& t : scenes[(size_t)i].tracks) {
                tracks[t] = shmy::audio::Player(AssetManager::get_stream(t));
                tracks[t].set_looping(true);
                tracks[t].start();
                tracks[t].fade(0.f, 1.f, FADETIME);
            }
        }

        portalgraph[scene.GetString()] = loader.get_portals();
        i++;
    }

    loader.load_gui();

    normal_mode.init(this);
    cinematic_mode.init(this);
    combat_mode.init(this);
}


void Game::queue_scene_swap(int index, sf::Vector2f pos) {
    if (index == active_scene) return;
    swap_queued = index;
    swap_pos = pos;
}

void Game::exec_scene_swap() {
    if (swap_queued < 0 || block_portals) {
        block_portals = false;
        swap_queued = -1;
        return;
    }
    Time::stop();
    auto& last = get_active_scene();
    active_scene = swap_queued;
    auto& scene = get_active_scene();
    scene.camera.setCenter(scene.world_to_screen.transformPoint(swap_pos));
    scene.background.update_soft(scene.camera.getFrustum());

    const auto FADETIME = 500;
    for (const auto& t : last.tracks) {
        if (!scene.tracks.contains(t)) {
            tracks[t].fade(0.f,  FADETIME);
            tracks[t].stop_after(FADETIME);
        }
    }
    for (const auto& t : scene.tracks) {
        if (!last.tracks.contains(t) || !tracks.contains(t)) {
            auto& track = tracks[t] = shmy::audio::Player(AssetManager::get_stream(t));
            track.set_looping(true);
            track.seek(0);
            track.start();
            track.stop_at(0);
            track.fade(0.f, 1.f, FADETIME);
        }
    }

    last.set_sleeping(true);
    scene.get_player().set_position(swap_pos, scene.world_to_screen);
    gui.get_widget<gui::Text>("area_label")->set_label(scene.name);
    block_portals = true;
    swap_queued = -1;
    scene.set_sleeping(false);
    Time::start();
}


void Game::update() {
    for (size_t i = 0; i < scenes.size(); i++) {
        scenes[i].update();
    }
    gui.update();
}

void Game::handle_events() {
    while (auto event = window->pollEvent()) {
        if (event->is<sf::Event::Closed>()) {
            window->close();
        } else if (const auto kyp = event->getIf<sf::Event::KeyPressed>()) {
            if (kyp->code == sf::Keyboard::Key::R && kyp->control) {
                const auto temp = get_active_scene_index();
                reload(module, temp);
            }
        } else if (const auto mmv = event->getIf<sf::Event::MouseMoved>()) {
            cursor.setPosition((sf::Vector2f)mmv->position);
        }

        get_active_scene().handle_event(*event);
    }
}

void Game::render(sf::RenderTarget& target) {
#ifdef SHMY_DEBUG
    target.setView(get_active_scene().camera);
    get_active_scene().debugger.render(target);
#endif

    target.setView(target.getDefaultView());
    target.draw(gui);
    target.draw(cursor);

#ifdef SHMY_DEBUG
    get_active_scene().debugger.render_ui(target);
#endif
}

