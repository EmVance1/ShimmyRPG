#include "pch.h"
#include "region.h"
#include "util/str.h"
#include "util/json.h"
#include "graphics/filters.h"


#define OUTLINE_WIDTH 5


void Region::load_from_file(const std::string& filename) {
    m_textures.clear();
    m_pathmaps.clear();
    m_alphamaps.clear();
    m_areas.clear();

    auto src = read_to_string(filename);
    rapidjson::Document doc;
    doc.Parse(src.data());

    const auto world = doc.GetObject()["world"].GetObject();
    auto _ = m_textures["bar_background_texture"].loadFromFile(world["background"].GetString());
    m_textures["bar_background_texture"].setSmooth(true);
    m_pathmaps["bar_pathmap"] = map_area(sf::Image(world["pathing"].GetString()), 3);

    for (const auto& [k, v] : doc.GetObject()["textures"].GetObject()) {
        const auto name = std::string(k.GetString());
        const auto texfile = v.GetObject()["file"].GetString();
        const auto outline = v.GetObject()["outlined"].IsTrue();
        const auto smooth  = v.GetObject()["smooth"].IsTrue();

        const auto img = sf::Image(texfile);
        m_alphamaps[name + "_map"].loadFromImage(img);
        _ = m_textures[name + "_texture"].loadFromImage(img);
        m_textures[name + "_texture"].setSmooth(smooth);
        if (outline) {
            _ = m_textures[name + "_outline"].loadFromImage((img.getSize().x < 200) ?
                                                            gen_outline(img, OUTLINE_WIDTH) :
                                                            gen_outline_threaded(img, OUTLINE_WIDTH));
            m_textures[name + "_outline"].setSmooth(smooth);
        }
    }

    m_areas.push_back(Area("bar", this, json_to_vector2f(world["topleft"]), world["scale"].GetFloat()));
    m_areas[0].init(doc);
}

