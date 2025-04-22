#pragma once
#include <SFML/Graphics.hpp>
#include <string>
#include <cstdint>
extern "C" {
#include <lua/lua.h>
#include <lua/lualib.h>
#include <lua/lauxlib.h>
}

struct Area;
class Entity;

class LuaScript {
public:
    enum class Callback {
        OnStart  = 1,
        OnUpdate = 2,
    };

    struct AsyncCallback {
        lua_State* thread = nullptr;
        bool resumable = false;
        float delay = 0;
    };

private:
    lua_State* m_state = nullptr;
    uint32_t m_funcs = 0;
    std::unordered_map<Callback, AsyncCallback> m_coroutines;

    Area* p_parent_area = nullptr;

public:
    LuaScript(Area& parent_area);
    LuaScript(Area& parent_area, const std::string& filename, bool autoplay = true);

    void load_from_file(const std::string& filename, bool autoplay = true);

    Area& parent_area() { return *p_parent_area; }
    const Area& parent_area() const { return *p_parent_area; }

    Entity& lookup_entity(const std::string& id);
    const Entity& lookup_entity(const std::string& id) const;

    void update();
};

