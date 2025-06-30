#include "pch.h"
#include "lua_script.h"
#include "lua_init.h"
#include "objects/entity.h"
#include "time/deltatime.h"
#include "world/area.h"
#include <stdexcept>


LuaScript::LuaScript(Area& parent_area) : p_parent_area(&parent_area) {}
LuaScript::LuaScript(Area& parent_area, const std::string& filename, bool autoplay)
    : p_parent_area(&parent_area)
{
    load_from_file(filename, autoplay);
}


static bool lua_hasfunction(lua_State* L, const char* name) {
    lua_getglobal(L, name);
    if (lua_isfunction(L, -1)) {
        lua_pop(L, 1);
        return true;
    } else {
        lua_pop(L, 1);
        return false;
    }
}


void LuaScript::load_from_file(const std::string& filename, bool autoplay) {
    m_state = luaL_newstate();
    luaL_openlibs(m_state);

    if (luaL_dofile(m_state, filename.c_str()) != LUA_OK) {
        std::cerr << "lua parse error - " << lua_tostring(m_state, -1) << "\n";
        exit(1);
    }

    if (lua_hasfunction(m_state, "OnUpdateAsync")) {
        luaL_dostring(m_state, "function OnUpdateAsyncImpl(deltatime) while true do OnUpdateAsync(deltatime) deltatime = shmy.yield() end end");
    }

    lua_pushstring(m_state, "script");
    lua_pushlightuserdata(m_state, this);
    lua_settable(m_state, LUA_REGISTRYINDEX);

    lua_register_engine_funcs(m_state);
    lua_set_overriden_funcs(m_state, m_funcs);
    lua_set_coroutines(m_state, m_coroutines);

    if (autoplay && (m_funcs & (1 << (uint32_t)Callback::OnStart))) {
        lua_getglobal(m_state, "OnStart");
#ifdef DEBUG
        if (lua_pcall(m_state, 0, 0, 0) != LUA_OK) {
            std::cerr << "lua pcall error - " << lua_tostring(m_state, -1) << "\n";
            exit(1);
        }
#else
        lua_call(m_state, 0, 0);
#endif
    }
    auto& async_start = m_coroutines[(size_t)Callback::OnStart];
    if (autoplay && async_start.resumable) {
        switch (lua_resume(async_start.thread, 0)) {
        case LUA_YIELD:
            if (const auto nresults = lua_gettop(async_start.thread); nresults == 1) {
                async_start.resumable = true;
                async_start.delay = static_cast<float>(lua_tonumber(async_start.thread, -1));
                lua_pop(async_start.thread, nresults);
            } else {
                std::cerr << "lua coroutine error - (internal) coroutine.yield nresults != 1\n";
                exit(1);
            }
            break;
        case LUA_OK:
            async_start.thread = nullptr;
            async_start.resumable = false;
            break;
        case LUA_ERRRUN:
            std::cerr << "lua coroutine error - " << lua_tostring(async_start.thread, -1) << "\n";
            exit(1);
        }
    }
}


Entity& LuaScript::lookup_entity(const std::string& id) {
    return p_parent_area->entities.at(p_parent_area->script_name_LUT.at(id));
}

const Entity& LuaScript::lookup_entity(const std::string& id) const {
    return p_parent_area->entities.at(p_parent_area->script_name_LUT.at(id));
}


void LuaScript::update() {
    const auto deltatime = Time::deltatime();

    if (m_funcs & (1 << (uint32_t)Callback::OnUpdate)) {
        lua_getglobal(m_state, "OnUpdate");
        lua_pushnumber(m_state, deltatime);
#ifdef DEBUG
        if (lua_pcall(m_state, 1, 0, 0) != LUA_OK) {
            std::cerr << "lua pcall error - " << lua_tostring(m_state, -1) << "\n";
            exit(1);
        }
#else
        lua_call(m_state, 1, 0);
#endif
    }

    auto& async_start = m_coroutines[(size_t)Callback::OnStart];
    if (async_start.resumable) {
        async_start.delay -= deltatime;
        if (async_start.delay <= 0) {
            switch (lua_resume(async_start.thread, 0)) {
            case LUA_YIELD:
                if (const auto nresults = lua_gettop(async_start.thread); nresults == 1) {
                    async_start.resumable = true;
                    async_start.delay = static_cast<float>(lua_tonumber(async_start.thread, -1));
                    lua_pop(async_start.thread, nresults);
                } else {
                    std::cerr << "lua coroutine error - (internal) coroutine.yield nresults != 1\n";
                    exit(1);
                }
                break;
            case LUA_OK:
                async_start.thread = nullptr;
                async_start.resumable = false;
                break;
            case LUA_ERRRUN:
                std::cerr << "lua coroutine error - " << lua_tostring(async_start.thread, -1) << "\n";
                exit(1);
            }
        }
    }

    auto& async_update = m_coroutines[(size_t)Callback::OnUpdate];
    if (async_update.resumable) {
        async_update.delay -= deltatime;
        if (async_update.delay <= 0) {
            lua_pushnumber(async_update.thread, deltatime);
            switch (lua_resume(async_update.thread, 1)) {
            case LUA_YIELD: {
                if (const auto nresults = lua_gettop(async_update.thread); nresults == 1) {
                    async_update.resumable = true;
                    async_update.delay = static_cast<float>(lua_tonumber(async_update.thread, -1));
                    lua_pop(async_update.thread, nresults);
                } else {
                    std::cerr << "lua coroutine error - (internal) coroutine.yield nresults != 1\n";
                    exit(1);
                }
                break; }
            case LUA_OK:
                async_update.thread = nullptr;
                async_update.resumable = false;
                break;
            case LUA_ERRRUN:
                std::cerr << "lua coroutine error - " << lua_tostring(async_update.thread, -1) << "\n";
                exit(1);
            }
        }
    }
}

