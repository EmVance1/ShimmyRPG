#include "pch.h"
#include "region.h"
#include "util/str.h"
#include "util/json.h"
#include "graphics/filters.h"
#include <stdio.h>


#define OUTLINE_WIDTH 5


void Region::load_from_folder(const std::string& folder) {
    m_textures.clear();
    m_pathmaps.clear();
    m_alphamaps.clear();
    m_areas.clear();

    m_guistyle.load_from_file("res/styles/wooden.json");

    auto src = read_to_string(folder + "region.json");
    rapidjson::Document doc;
    doc.Parse(src.data());

    for (const auto& [k, v] : doc.GetObject()["textures"].GetObject()) {
        const auto name = std::string(k.GetString());
        const auto texfile = v.GetObject()["file"].GetString();
        const auto outline = v.GetObject()["outlined"].IsTrue();
        const auto smooth  = v.GetObject()["smooth"].IsTrue();
        const auto looped  = v.GetObject()["looped"].IsTrue();

        const auto img = sf::Image(texfile);
        m_alphamaps[name + "_map"].loadFromImage((img.getSize().x < 200) ?
                                                gen_clickmap(img, OUTLINE_WIDTH) :
                                                gen_clickmap_threaded(img, OUTLINE_WIDTH));
        auto _ = m_textures[name + "_texture"].loadFromImage(img);
        m_textures[name + "_texture"].setSmooth(smooth);
        m_textures[name + "_texture"].setRepeated(looped);
        if (outline) {
            _ = m_textures[name + "_outline"].loadFromImage((img.getSize().x < 200) ?
                                                            gen_outline(img, OUTLINE_WIDTH) :
                                                            gen_outline_threaded(img, OUTLINE_WIDTH));
            m_textures[name + "_outline"].setSmooth(smooth);
            m_textures[name + "_outline"].setRepeated(looped);
        }
    }

    for (const auto& area : doc.GetObject()["areas"].GetArray()) {
        const auto area_file = std::string(area.GetObject()["file"].GetString());

        auto _ = m_textures[area_file + "_background_texture"].loadFromFile(area.GetObject()["background"].GetString());
        m_textures[area_file + "_background_texture"].setSmooth(true);
        m_pathmaps[area_file + "_pathmap"] = map_area(sf::Image(area.GetObject()["pathing"].GetString()), 3);

        m_areas.emplace_back(area_file, this, json_to_vector2f(area.GetObject()["topleft"]), area.GetObject()["scale"].GetFloat());
    }
    size_t i = 0;
    for (const auto& area : doc.GetObject()["areas"].GetArray()) {
        const auto area_file = std::string(area.GetObject()["file"].GetString());

        FILE* fp = nullptr;
        fopen_s(&fp, (folder + area_file + ".json").c_str(), "rb");
        char readBuffer[65536];
        rapidjson::FileReadStream is(fp, readBuffer, sizeof(readBuffer));
        rapidjson::Document area_doc;
        area_doc.ParseStream(is);
        fclose(fp);

        // auto area_src = read_to_string(folder + area_file + ".json");
        // rapidjson::Document area_doc;
        // area_doc.Parse(area_src.data());

        m_areas[i].init(area_doc);
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

