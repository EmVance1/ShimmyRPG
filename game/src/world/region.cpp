#include "pch.h"
#include "region.h"
#include "graphics/filters.h"
#include "util/deltatime.h"
#include "util/json.h"
#include "util/env.h"
#include "io/load_scene.h"


#define OUTLINE_WIDTH 5


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
    m_guistyle.load_from_dir(shmy::env::pkg_full() / "gui" / doc["gui_style"].GetString());
    std::cout << "2 - areas: " << m_areas.size() << ", active: " << m_active_area << ", passed in: " << initial_area << "\n";
#endif

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
        if (i != m_active_area) {
            m_areas[i].set_mode(GameMode::Sleep);
        }
        i++;
    }
}

void Region::set_active_area(size_t index) {
    Time::stop();
    const auto mode = m_areas[m_active_area].gamemode;
    m_areas[m_active_area].set_mode(GameMode::Sleep);
    m_areas[index].set_mode(mode);
    m_active_area = index;
    Time::start();
}

void Region::update_all() {
    for (size_t i = 0; i < m_areas.size(); i++) {
        m_areas[i].update();
    }
}

