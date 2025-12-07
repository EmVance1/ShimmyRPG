#pragma once
#include <sfutil/sfutil.h>
#include <unordered_map>
#include <string>
#include "area.h"


struct RenderSettings {
    const sf::Vector2i viewport;
    std::vector<sf::Shader> shaders;
    sf::FloatRect crop;
    sf::Color overlay;

    RenderSettings(const sf::Vector2i& _viewport)
        : viewport(_viewport), crop({0, 0}, (sf::Vector2f)viewport), overlay(sf::Color::White)
    {}
};


class Region {
public:
    inline static RenderSettings* render_settings = nullptr;

private:
    std::string m_id;
    std::unordered_map<std::string, sfu::TextureAtlas> m_atlases;
    std::unordered_map<std::string, sf::Texture> m_textures;
    std::unordered_map<std::string, sfu::AlphaMap> m_alphamaps;
    std::vector<Area> m_areas;
    gui::Style m_guistyle;
    size_t m_active_area = 0;

    NormalMode normal_mode;
    CinematicMode cinematic_mode;
    CombatMode combat_mode;
    GameMode gamemode = GameMode::Normal;

    int swap_queued = -1;
    sf::Vector2f swap_pos;
    bool block_portals = false;

public:
    gui::Panel m_gui;
    sf::Sprite* p_cursor;

public:
    Region();

    void load_from_dir(const std::filesystem::path& folder, size_t initial_area);

    void queue_scene_swap(int index, sf::Vector2f pos);
    void exec_scene_swap();

    Area& get_active_area() { return m_areas[m_active_area]; }
    size_t get_active_area_index() const { return m_active_area; };

    const gui::Style& style() const { return m_guistyle; }
    const std::string& id() const { return m_id; }

    void update_all();

    friend struct Area;
    friend class SceneLoader;
    friend class AreaDebugger;
};

