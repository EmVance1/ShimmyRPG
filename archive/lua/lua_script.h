#pragma once
#include <SFML/Graphics.hpp>
#include <string>
#include <lua/lua.h>
#include <lua/lualib.h>
#include <lua/lauxlib.h>
#include <cstdint>


class Script {
public:
    enum class Callback {
        None           = 0,
        OnCreate       = 1 << 0,
        OnStart        = 1 << 1,
        OnMouseClick   = 1 << 2,
        OnMouseRelease = 1 << 3,
        OnMouseMove    = 1 << 4,
        OnMouseScroll  = 1 << 5,
        OnKeyPress     = 1 << 6,
        OnKeyRelease   = 1 << 7,
        OnUpdate       = 1 << 8,
    };

    struct AsyncCallback {
        lua_State* thread = nullptr;
        bool resumable = false;
        float delay = 0;
    };

private:
    lua_State* m_state;
    uint32_t m_funcs;
    std::unordered_map<Callback, AsyncCallback> m_coroutines;

private:
    void callbacks(const sf::Event& event);
    void async_callbacks(const sf::Event& event);

public:
    Script(const std::string& filename);

    void load_from_file(const std::string& filename);

    void handle_event(const sf::Event& event);
    void update(float deltatime);
};

