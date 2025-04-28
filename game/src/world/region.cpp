#include "pch.h"
#include "region.h"
#include "util/str.h"
#include "util/json.h"
#include "graphics/filters.h"


#define OUTLINE_WIDTH 5


void Region::load_from_folder(const std::string& folder) {
    m_textures.clear();
    m_pathmaps.clear();
    m_alphamaps.clear();
    m_areas.clear();

    m_guistyle.load_from_file("res/styles/wooden.json");
    const auto doc = load_json_from_file(folder + "region.json");

    for (const auto& [k, v] : doc.GetObject()["textures"].GetObject()) {
        const auto name = std::string(k.GetString());
        const auto texfile = v.GetObject()["file"].GetString();
        const auto outline = v.GetObject()["outlined"].IsTrue();
        const auto smooth  = v.GetObject()["smooth"].IsTrue();
        const auto dims    = json_to_vector2u(v.GetObject()["dims"]);

        const auto img = sf::Image(texfile);
        m_alphamaps[name + "_map"].loadFromImage(
                (img.getSize().x < 200) ? gen_clickmap(img, OUTLINE_WIDTH) : gen_clickmap_threaded(img, OUTLINE_WIDTH)
            );
        auto _ = m_atlases[name].loadFromImage(img, dims);
        m_atlases[name].setSmooth(smooth);
        if (!outline) { continue; }
        _ = m_atlases[name + "_outline"].loadFromImage(
                (img.getSize().x < 200) ? gen_outline(img, OUTLINE_WIDTH) : gen_outline_threaded(img, OUTLINE_WIDTH), dims
            );
        m_atlases[name + "_outline"].setSmooth(smooth);
    }

    const auto prefabs = load_json_from_file("res/prefabs.json");
    for (const auto& area : doc.GetObject()["areas"].GetArray()) {
        const auto area_file = std::string(area.GetObject()["file"].GetString());
        m_pathmaps[area_file + "_pathmap"] = map_area(sf::Image(area.GetObject()["pathing"].GetString()), 3);
        m_areas.emplace_back(area_file, this, json_to_vector2f(area.GetObject()["topleft"]), area.GetObject()["scale"].GetFloat());
    }
    size_t i = 0;
    for (const auto& area : doc.GetObject()["areas"].GetArray()) {
        const auto area_file = std::string(area.GetObject()["file"].GetString());
        const auto area_doc = load_json_from_file(folder + area_file + ".json");;
        m_areas[i].init(prefabs, area_doc);
        m_areas[i].set_mode(GameMode::Sleep, false);
        i++;
    }
}

void Region::set_active_area(size_t index) {
    m_areas[m_active_area].set_mode(GameMode::Sleep, false);
    m_areas[index].set_mode(GameMode::Normal, false);
    m_active_area = index;
}

void Region::update_all() {
    for (size_t i = 0; i < m_areas.size(); i++) {
        m_areas[i].update();
    }
}

