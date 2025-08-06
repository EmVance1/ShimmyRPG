#include "pch.h"
#include "region.h"
#include "util/str.h"
#include "util/json.h"
#include "util/env.h"
#include "graphics/filters.h"
#include "time/deltatime.h"


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

    m_guistyle.load_from_dir(shmy::env::pkg_full() / "gui" / doc["gui_style"].GetString());

    const auto& areas = doc["areas"].GetArray();
    m_areas.reserve(areas.Size());
    const auto prefabs = shmy::json::load_from_file(shmy::env::pkg_full() / "prefabs.json");
    for (const auto& area : areas) {
        m_areas.emplace_back(area.GetString(), this);
    }
    size_t i = 0;
    for (const auto& area : areas) {
#ifdef DEBUG
        try {
            const auto area_doc = shmy::json::load_from_file(shmy::env::pkg_full() / folder / (std::string(area.GetString()) + ".json"));
            m_areas[i].init(prefabs, area_doc);
        } catch (const std::exception& e) {
            std::cerr << "error loading area '" << m_areas[i].id << "': " << e.what() << "\n";
            exit(1);
        }
#else
        const auto area_doc = shmy::json::load_from_file(shmy::env::pkg_full() / folder / (std::string(area.GetString()) + ".json"));
        m_areas[i].init(prefabs, area_doc);
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

