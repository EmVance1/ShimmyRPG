#include "pch.h"
#include "region.h"
#include "util/str.h"
#include "util/json.h"
#include "graphics/filters.h"
#include "time/deltatime.h"


#define OUTLINE_WIDTH 5


void Region::load_from_folder(const std::string& folder) {
    m_textures.clear();
    m_alphamaps.clear();
    m_areas.clear();

    const auto doc = load_json_from_file(folder + "region.json");

    m_id = folder;

    for (const auto& [k, v] : doc["textures"].GetObject()) {
        const auto name = std::string(k.GetString());
        const auto texfile = v["file"].GetString();
        const auto outline = v["outlined"].IsTrue();
        const auto smooth  = v["smooth"].IsTrue();
        const auto dims    = json_to_vector2u(v["dims"]);

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

    m_guistyle.load_from_file(std::string("res/styles/") + doc["gui_style"].GetString() + ".json");

    const auto prefabs = load_json_from_file("res/prefabs.json");
    for (const auto& area : doc["areas"].GetArray()) {
        const auto area_file = std::string(area["file"].GetString());
        m_areas.emplace_back(area_file, this);
    }
    size_t i = 0;
    for (const auto& area : doc["areas"].GetArray()) {
        const auto area_file = std::string(area["file"].GetString());
#ifdef DEBUG
        try {
            const auto area_doc = load_json_from_file(folder + area_file + ".json");;
            m_areas[i].init(prefabs, area_doc);
        } catch (const std::exception& e) {
            std::cerr << "error loading area '" << m_areas[i].id << "': " << e.what() << "\n";
            exit(1);
        }
#else
        const auto area_doc = load_json_from_file(folder + area_file + ".json");;
        m_areas[i].init(prefabs, area_doc);
#endif
        m_areas[i].set_mode(GameMode::Sleep);
        i++;
    }
}

void Region::set_active_area(size_t index) {
    m_areas[m_active_area].set_mode(GameMode::Sleep);
    m_areas[index].set_mode(GameMode::Normal);
    m_active_area = index;
    Time::set_frame();
}

void Region::update_all() {
    for (size_t i = 0; i < m_areas.size(); i++) {
        m_areas[i].update();
    }
}

