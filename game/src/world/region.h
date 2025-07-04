#pragma once
#include <unordered_map>
#include <string>
#include <sfutil/sfutil.h>
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
    void load_from_folder(const std::string& folder, size_t initial_area);

    Area& get_active_area() { return m_areas[m_active_area]; }
    size_t get_active_area_index() const { return m_active_area; };
    void set_active_area(size_t index);

    const gui::Style& get_style() const { return m_guistyle; }

    void update_all();

    friend struct Area;
    friend class AreaDebugView;
};

