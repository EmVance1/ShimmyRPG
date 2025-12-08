#include "pch.h"
#include "region.h"
#include "graphics/filters.h"
#include "util/deltatime.h"
#include "util/json.h"
#include "util/env.h"
#include "io/init/load_scene.h"


#define OUTLINE_WIDTH 5

Region::Region() :
    m_gui(gui::Position::topleft({0, 0}), sf::Vector2f(render_settings->viewport), gui::Style())
{}

void Region::load_from_dir(const std::fs::path& folder, size_t initial_area) {
    m_textures.clear();
    m_alphamaps.clear();
    m_areas.clear();

    const auto doc = shmy::json::load_from_file(shmy::env::pkg_full() / folder / "region.json");

    m_id = folder.string();
    m_active_area = initial_area;

    for (const auto& [k, v] : doc["textures"].GetObject()) {
        const auto name = std::string(k.GetString());
        const auto texfile = shmy::env::pkg_full() / v["file"].GetString();
        const auto outline = v["outlined"].IsTrue();
        const auto smooth  = v["smooth"].IsTrue();
        const auto dims    = shmy::json::into_vector2u(v["dims"]);

        const auto img = sf::Image(texfile);
        m_alphamaps[name + "_map"].loadFromImage(
                (img.getSize().x < 200) ? shmy::filter::clickmap(img, OUTLINE_WIDTH) : shmy::filter::clickmap_threaded(img, OUTLINE_WIDTH)
            );
        std::ignore = m_atlases[name].loadFromImage(img, dims);
        m_atlases[name].setSmooth(smooth);
        if (!outline) { continue; }
        std::ignore = m_atlases[name + "_outline"].loadFromImage(
                (img.getSize().x < 200) ? shmy::filter::outline(img, OUTLINE_WIDTH) : shmy::filter::outline_threaded(img, OUTLINE_WIDTH), dims
            );
        m_atlases[name + "_outline"].setSmooth(smooth);
    }

#ifdef _WIN32
    m_guistyle.load_from_dir(shmy::env::pkg_full() / "gui" / doc["gui_style"].GetString());
#else
    std::cout << "1 - areas: " << m_areas.size() << ", active: " << m_active_area << ", passed in: " << initial_area << "\n";
    // m_guistyle.load_from_dir(shmy::env::pkg_full() / "gui" / doc["gui_style"].GetString());
    std::cout << "2 - areas: " << m_areas.size() << ", active: " << m_active_area << ", passed in: " << initial_area << "\n";
#endif

    m_gui.set_style(m_guistyle);
    m_gui.set_size((sf::Vector2f)render_settings->viewport);
    m_gui.set_background_color(sf::Color::Transparent);

    const auto& areas = doc["areas"].GetArray();
    const auto prefabs = shmy::json::load_from_file(shmy::env::pkg_full() / "prefabs.json");
    m_areas.resize(areas.Size());
    auto loader = SceneLoader(this, prefabs);

    size_t i = 0;
    for (const auto& area : areas) {
#ifdef VANGO_DEBUG
        try {
            loader.load(&m_areas[i], area.GetString());
        } catch (const std::exception& e) {
            std::cerr << "load module error - area '" << area.GetString() << "': " << e.what() << "\n";
            exit(1);
        }
#else
        loader.load(&m_areas[i], area.GetString());
#endif
        if (i == m_active_area) {
            m_areas[i].set_sleeping(false);
        }
        i++;
    }

    loader.load_gui();

    normal_mode.init(this);
    cinematic_mode.init(this);
    combat_mode.init(this);
}


void Region::queue_scene_swap(int index, sf::Vector2f pos) {
    if (index == m_active_area) return;
    swap_queued = index;
    swap_pos = pos;
}

void Region::exec_scene_swap() {
    if (swap_queued < 0 || block_portals) {
        block_portals = false;
        swap_queued = -1;
        return;
    }
    Time::stop();
    auto& last = m_areas[m_active_area];
    m_active_area = swap_queued;
    auto& area = get_active_area();
    area.camera.setCenter(area.cart_to_iso.transformPoint(swap_pos));
    area.background.update_soft(area.camera.getFrustum());
    last.set_sleeping(true);
    area.get_player().set_position(swap_pos, area.cart_to_iso);
    m_gui.get_widget<gui::Text>("area_label")->set_label(area.name);
    block_portals = true;
    swap_queued = -1;
    area.set_sleeping(false);
    Time::start();
}

void Region::update_all() {
    for (size_t i = 0; i < m_areas.size(); i++) {
        m_areas[i].update();
    }
    m_gui.update();
}

