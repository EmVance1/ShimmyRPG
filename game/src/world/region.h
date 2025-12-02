#pragma once
#include <sfutil/sfutil.h>
#include <unordered_map>
#include <string>
#include "area.h"


class Region {
private:
    std::string m_id;
    std::unordered_map<std::string, sfu::TextureAtlas> m_atlases;
    std::unordered_map<std::string, sf::Texture> m_textures;
    std::unordered_map<std::string, sfu::AlphaMap> m_alphamaps;
    std::vector<Area> m_areas;
    gui::Style m_guistyle;
    size_t m_active_area = 0;

public:
    void load_from_dir(const std::filesystem::path& folder, size_t initial_area);

    Area& get_active_area() { return m_areas[m_active_area]; }
    size_t get_active_area_index() const { return m_active_area; };
    void set_active_area(size_t index);

    const gui::Style& style() const { return m_guistyle; }
    const std::string& id() const { return m_id; }

    void update_all();

    friend struct Area;
    friend class SceneLoader;
    friend class AreaDebugger;
};

