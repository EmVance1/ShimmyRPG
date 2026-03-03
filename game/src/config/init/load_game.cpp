#include "pch.h"
#include "core/split.h"
#include "util/env.h"
#include "util/json.h"
#include "world/game.h"
#include "data/flags.h"
#include "data/bundler.h"
#include "data/mixer.h"
#include "config/init/load_scene.h"
#include <filesystem>


Viewport::Viewport(const sf::Vector2u& win) {
    constexpr auto REF = sf::Vector2i(1920, 1080);
    constexpr auto FREF = (sf::Vector2f)REF;
    constexpr auto ASPECT = 16.f / 9.f;
    const auto aspect = (float)win.x / (float)win.y;
    const auto ratio = aspect / ASPECT;
    if (ratio > 1.1f) {        // screen much shorter/wider than frame
        scale = (float)win.y / FREF.y;
        box = sf::IntRect{ { (int)((float)win.x - FREF.x * scale) / 2, 0 }, REF };
    } else if (ratio < 0.9f) {                  // screen much taller/narrower than frame
        scale = (float)win.x / FREF.x;
        box = sf::IntRect{ { 0, (int)((float)win.y - FREF.y * scale) / 2 }, REF };
    } else if (ratio > 1.f) {  // screen a little shorter/wider than frame
        scale = (float)win.y / FREF.y;
        box = sf::IntRect{ { 0, 0 }, sf::Vector2i((int)(FREF.y * aspect), REF.y) };
    } else {                   // screen a little taller/narrower than frame
        scale = (float)win.x / FREF.x;
        box = sf::IntRect{ { 0, 0 }, sf::Vector2i(REF.x, (int)(FREF.x / aspect)) };
    }
}


static void arrival_callback(void* ptr);

void Game::reload() {
    reload(m_module, (int)m_active_scene);
}

void Game::reload(const std::fs::path& _module, int start_area) {
    render_ctx.shaders.clear();
    m_scenes.clear();
    m_scene_map.clear();
    m_entities.clear();
    m_entity_map.clear();
    m_active_player = UINT32_MAX;
    shmy::data::Flags::reset();
    shmy::data::Bundler::reset();
    shmy::data::Mixer::reset();

    m_module = _module;
    shmy::env::set_pkg(m_module);
    const auto modfile = shmy::json::load_from_file(shmy::env::pkg_full() / "module.json");
    const auto initials = shmy::core::split(modfile["init_scene"].GetString(), '/');
    m_region = initials[0];
    const auto init_scene = initials[1];

    try {
        shmy::data::Flags::init(shmy::env::pkg_full() / "flags");
        shmy::data::Bundler::init(shmy::env::pkg_full() / "bundles", p_viewport->box.size);
    } catch (const std::exception& e) {
        std::cerr << "load module error - " << e.what() << "\n";
        exit(1);
    }

    if (!style.load_from_dir(shmy::env::pkg_full() / "gui" / modfile["gui_style"].GetString())) {
        std::cerr << "error loading GUI style\n";
        exit(1);
    }
    gui.set_style(style);
    gui.set_sizing((sf::Vector2f)render_ctx.letterbox.size);
    gui.set_background_color(sf::Color::Transparent);
    cursor.setTexture(style.cursor_texture, true);

    auto loader = SceneLoader(this, modfile["characters"], modfile["props"]);

    for (auto e : std::fs::directory_iterator(shmy::env::pkg_full() / "world" / m_region)) {
        if (e.is_directory()) {
            m_scene_map[e.path().filename().string()] = 0;
        }
    }
    size_t scene_count = 0;
    for (auto& [name, idx] : m_scene_map) {
        idx = scene_count++;
    }
    m_scenes.reserve(scene_count);
    for (auto& [name, idx] : m_scene_map) {
        Scene* ptr;
#ifdef SHMY_DEBUG
        try {
            ptr = &m_scenes.emplace_back(loader, m_region, name);
        } catch (const std::exception& e) {
            std::cerr << "load module error - scene '" << name << "': " << e.what() << "\n";
            exit(1);
        }
#else
        ptr = &m_scenes.emplace_back(loader, m_region, name);
#endif
        // if ((start_area < 0 && key == initial_scene) || (start_area >= 0 && idx == (size_t)start_area)) {
        if (name == init_scene) {
            m_active_scene = idx;
            ptr->set_sleeping(false);
            ptr->camera.setCenter(player().get_sprite().getPosition(), true);
            for (const auto& t : ptr->tracks) {
                auto opts = shmy::data::Mixer::StartOptions();
                opts.loop = true;
                opts.fadein_vol = 1.f;
                opts.fadein_millis = 500;
                shmy::data::Mixer::play_track(t, opts);
            }
        }
    }

    if (m_active_player == UINT32_MAX) {
        std::cerr << "exactly one entity MUST be designated 'active = true'\n";
        exit(1);
    }

    shmy::data::Bundler::require("postfx");
    for (const auto& [_, v] : shmy::data::Bundler::get_bundle("postfx").get_shaders()) {
        render_ctx.shaders.push_back(&v);
    }

    active_scene().init_gui(gui);

    normal_mode.init(this);
    cinematic_mode.init(this);

    for (auto& e : m_entities) {
        if (e.is_character()) {
            e.get_tracker().set_arrival_callback(arrival_callback, &e);
        }
    }

#ifdef SHMY_DEBUG
    m_debugger.init(&m_scenes[m_active_scene]);
#endif
}


static void arrival_callback(void* ptr) {
    auto e = (Entity*)ptr;
    e->busy_reached(Entity::BusyTarget::DestReached);
}

