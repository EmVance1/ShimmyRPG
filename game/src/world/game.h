#pragma once
#include <sfutil/sfutil.h>
#include <audio/lib.h>
#include <unordered_map>
#include <string>
#include "scene.h"


struct RenderSettings {
    const sf::Vector2i viewport;
    std::vector<sf::Shader> shaders;
    sf::FloatRect crop;
    sf::Color overlay;

    RenderSettings(const sf::Vector2i& _viewport)
        : viewport(_viewport), crop({0, 0}, (sf::Vector2f)viewport), overlay(sf::Color::White)
    {}
};


class Game {
public:
    sf::RenderWindow* window;
    RenderSettings render_settings;

private:
    std::vector<Scene> scenes;
    int active_scene = 0;
    std::unordered_map<std::string, std::vector<size_t>> portalgraph;
    std::filesystem::path module;
    std::string region;

    int swap_queued = -1;
    sf::Vector2f swap_pos;
    bool block_portals = false;

    NormalMode normal_mode;
    CinematicMode cinematic_mode;
    CombatMode combat_mode;
    GameMode gamemode = GameMode::Normal;

    std::unordered_map<std::string, shmy::audio::Player> tracks;

public:
    gui::Panel gui;
    gui::Style style;
    sf::Sprite cursor;

public:
    Game(sf::RenderWindow* window);

    void reload(const std::filesystem::path& module, int start_area = -1);

    void queue_scene_swap(int index, sf::Vector2f pos);
    void exec_scene_swap();

    Scene& get_active_scene() { return scenes[(size_t)active_scene]; }
    int get_active_scene_index() const { return active_scene; };

    void update();
    void handle_events();
    void render(sf::RenderTarget& target);

    friend struct Scene;
    friend class SceneLoader;
    friend class SceneDebugger;
};

