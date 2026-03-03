#pragma once
#include <sfutil/sfutil.h>
#include <audio/lib.h>
#include <unordered_map>
#include <string>
#include "game/simulate.h"
#include "game/cinematic.h"
#include "game/event.h"
#include "objects/entity.h"
#include "scene.h"
#ifdef SHMY_DEBUG
#include "debugger.h"
#endif


struct Viewport {
    sf::IntRect box;
    float scale;

    Viewport(const sf::Vector2u& window);
};
struct RenderContext {
    sf::RenderWindow* window;
    sf::FloatRect letterbox;
    bool cinematic_wide = false;
    sf::Color overlay = sf::Color::White;
    std::vector<const sf::Shader*> shaders = {};
};


namespace shmy::sim {

}

class Game {
private:
    std::filesystem::path m_module;
    std::string m_region = "";

    std::vector<Scene> m_scenes;
    std::unordered_map<std::string, size_t> m_scene_map;
    size_t m_active_scene = 0;

    std::vector<Entity> m_entities;
    std::unordered_map<std::string, uint32_t> m_entity_map;
    uint32_t m_active_player = UINT32_MAX;

    const Portal* m_portal_queue = nullptr;
    uint32_t m_portal_entity;

    Viewport* p_viewport;
    shmy::EventQueue m_eventqueue;

#ifdef SHMY_DEBUG
    Debugger m_debugger;
#endif

    static Game* emergencies_only;

public:
    enum class Mode {
        Simulation = 0,
        Cinematic = 1,
    };
    RenderContext render_ctx;

    shmy::sim::Simulation normal_mode;
    shmy::sim::Cinematic cinematic_mode;
    Mode gamemode = Mode::Simulation;

    gui::Panel gui;
    gui::Style style;
    sf::Sprite cursor;
    sf::Vector2f mouse_now;
    uint32_t hover_entity = UINT32_MAX;

public:
    Game(sf::RenderWindow* window, Viewport& viewport);
    ~Game();

    void reload();
    void reload(const std::filesystem::path& module, int start_area = -1);

    Entity& entity(uint32_t handle);
    const Entity& entity(uint32_t handle) const;

    Entity& entity_by_id(const std::string& id);
    const Entity& entity_by_id(const std::string& id) const;
    uint32_t entity_handle(const std::string& id) const;
    static uint32_t entity_hook(const char* id);

    Entity& player();
    const Entity& player() const;
    uint32_t player_id() const;

    void set_mode(Mode mode, bool cinema_manual = false, bool cinema_wide = false);
    void queue_portal(const Portal& portal, uint32_t entity);
    void exec_scene_swap();

    Scene& active_scene() { return m_scenes[m_active_scene]; }
    size_t active_scene_index() const { return m_active_scene; };

    void handle_events();
    void update();
    void render(sf::RenderWindow& window, sf::RenderTexture* ping, sf::RenderTexture* pong);

    // friend struct Scene;
    friend class SceneLoader;
};

