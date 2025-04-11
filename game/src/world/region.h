#pragma once
#include <unordered_map>
#include <string>
#include "area.h"


class Region {
private:
    std::unordered_map<std::string, sf::Texture> m_textures;
    std::unordered_map<std::string, sf::Image> m_pathmaps;
    std::unordered_map<std::string, sfu::AlphaMap> m_alphamaps;
    std::vector<Area> m_areas;
    size_t m_active_area = 0;

public:
    void load_from_file(const std::string& filename);

    Area& active_area() { return m_areas[m_active_area]; }

    friend class Area;
};

