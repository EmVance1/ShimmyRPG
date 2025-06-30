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


void LuaScript::load_from_file(const std::string& filename, bool autoplay) {
    m_state = luaL_newstate();
    luaL_openlibs(m_state);

    if (luaL_dofile(m_state, filename.c_str()) == LUA_OK) {
        lua_pushstring(m_state, "script");
        lua_pushlightuserdata(m_state, this);
        lua_settable(m_state, LUA_REGISTRYINDEX);

        lua_register_engine_funcs(m_state);
        lua_set_overriden_funcs(m_state, m_funcs);
        lua_set_coroutines(m_state, m_coroutines);

        if (autoplay && (m_funcs & (uint32_t)Callback::OnStart)) {
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
        auto& async_start= m_coroutines[Callback::OnStart];
        if (autoplay && async_start.callable) {
            async_start.thread = lua_newthread(m_state);
            async_start.resumable = true;
            async_start.delay = 0.f;
            lua_getglobal(async_start.thread, "OnStartAsync");
            lua_pop(m_state, -1);

            const auto resume_result = lua_resume(async_start.thread, 0);
            if (resume_result == LUA_YIELD) {
                async_start.resumable = true;
                const int nresults = lua_gettop(async_start.thread);
                if (nresults == 1) {
                    async_start.delay = static_cast<float>(lua_tonumber(async_start.thread, -1));
                    lua_pop(async_start.thread, nresults);
                } else {
#ifdef DEBUG
                    std::cerr << "lua coroutine error - (internal) coroutine.yield nresults != 1\n";
                    exit(1);
#endif
                }
            } else if (resume_result == LUA_ERRRUN) {
#ifdef DEBUG
                std::cerr << "lua coroutine error - " << lua_tostring(async_start.thread, -1) << "\n";
                exit(1);
#endif
            } else {
                async_start.resumable = false;
            }
        }
    } else {
#ifdef DEBUG
        std::cerr << "lua parse error - " << lua_tostring(m_state, -1) << "\n";
        exit(1);
#endif
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

    if (m_funcs & (uint32_t)Callback::OnUpdate) {
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

    auto& async_update = m_coroutines[Callback::OnUpdate];
    if (!async_update.thread && async_update.callable) {
        async_update.thread = lua_newthread(m_state);
        async_update.resumable = true;
        async_update.delay = 0.f;
        lua_getglobal(async_update.thread, "OnUpdateAsync");
        lua_pop(m_state, -1);
    }
    async_update.delay -= deltatime;
    if (async_update.delay <= 0 && async_update.resumable) {
        lua_pushnumber(async_update.thread, deltatime);
        const auto resume_result = lua_resume(async_update.thread, 1);
        if (resume_result == LUA_YIELD) {
            async_update.resumable = true;
            const int nresults = lua_gettop(async_update.thread);
            if (nresults == 1) {
                async_update.delay = static_cast<float>(lua_tonumber(async_update.thread, -1));
                lua_pop(async_update.thread, nresults);
            } else {
#ifdef DEBUG
                std::cerr << "lua coroutine error - (internal) coroutine.yield nresults != 1\n";
                exit(1);
#endif
            }
        } else if (resume_result == LUA_ERRRUN) {
#ifdef DEBUG
            std::cerr << "lua coroutine error - " << lua_tostring(async_update.thread, -1) << "\n";
            exit(1);
#endif
        } else {
            async_update.thread = nullptr;
            async_update.resumable = false;
        }
    }

    auto& async_start = m_coroutines[Callback::OnStart];
    async_start.delay -= deltatime;
    if (async_start.delay <= 0 && async_start.resumable) {
        const auto resume_result = lua_resume(async_start.thread, 0);
        if (resume_result == LUA_YIELD) {
            async_start.resumable = true;
            const int nresults = lua_gettop(async_start.thread);
            if (nresults == 1) {
                async_start.delay = static_cast<float>(lua_tonumber(async_start.thread, -1));
                lua_pop(async_start.thread, nresults);
            } else {
#ifdef DEBUG
                std::cerr << "lua coroutine error - (internal) coroutine.yield nresults != 1\n";
                exit(1);
#endif
            }
        } else if (resume_result == LUA_ERRRUN) {
#ifdef DEBUG
            std::cerr << "lua coroutine error - " << lua_tostring(async_start.thread, -1) << "\n";
            exit(1);
#endif
        } else {
            async_start.thread = nullptr;
            async_start.resumable = false;
        }
    }
}

