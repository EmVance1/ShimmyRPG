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
    std::cout << "creating script\n";
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
            if (lua_pcall(m_state, 0, 0, 0) != 0) {
                // throw std::invalid_argument(std::string("lua pcall error - ") + lua_tostring(m_state, -1));
            }
#else
            lua_call(m_state, 0, 0);
#endif
        }
        auto& asfunc = m_coroutines[Callback::OnStart];
        if (autoplay && asfunc.thread) {
            const auto resume_result = lua_resume(asfunc.thread, 0);
            if (resume_result == LUA_YIELD) {
                asfunc.resumable = true;
                const int nresults = lua_gettop(asfunc.thread);
                if (nresults == 1) {
                    asfunc.delay = static_cast<float>(lua_tonumber(asfunc.thread, -1));
                    lua_pop(asfunc.thread, nresults);
                } else {
                    std::cerr << "lua coroutine error - (internal) coroutine.yield nresults != 1\n";
                    exit(1);
                }
            } else if (resume_result == LUA_ERRRUN) {
                std::cerr << "lua coroutine error - " << lua_tostring(asfunc.thread, -1) << "\n";
                exit(1);
            } else {
                asfunc.resumable = false;
            }
        }
    } else {
        throw std::invalid_argument(std::string("lua parse error - ") + lua_tostring(m_state, -1));
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
        if (lua_pcall(m_state, 1, 0, 0) != 0) {
            // throw std::invalid_argument(std::string("lua pcall error - ") + lua_tostring(m_state, -1));
        }
#else
        lua_call(m_state, 1, 0);
#endif
    }
    auto& asfunc = m_coroutines[Callback::OnUpdate];
    if (asfunc.thread && asfunc.delay <= 0) {
        lua_pushnumber(asfunc.thread, deltatime);
        asfunc.resumable = lua_resume(asfunc.thread, 1) == LUA_YIELD;
        const int nresults = lua_gettop(asfunc.thread);
        if (nresults > 0) {
            asfunc.delay = static_cast<float>(lua_tonumber(asfunc.thread, -1));
            lua_pop(asfunc.thread, nresults);
        }
    }
    for (auto& [id, co] : m_coroutines) {
        // if (id == Callback::OnUpdate) { continue; }
        co.delay -= deltatime;
        if (co.delay <= 0 && co.resumable) {
            const auto resume_result = lua_resume(co.thread, 0);
            if (resume_result == LUA_YIELD) {
                co.resumable = true;
                const int nresults = lua_gettop(co.thread);
                if (nresults == 1) {
                    co.delay = static_cast<float>(lua_tonumber(co.thread, -1));
                    lua_pop(co.thread, nresults);
                } else {
                    std::cerr << "lua coroutine error - (internal) coroutine.yield nresults != 1\n";
                    exit(1);
                }
            } else if (resume_result == LUA_ERRRUN) {
                std::cerr << "lua coroutine error - " << lua_tostring(co.thread, -1) << "\n";
                exit(1);
            } else {
                co.resumable = false;
            }
        }
    }
}

