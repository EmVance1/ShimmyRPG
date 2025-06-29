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
            if (lua_pcall(m_state, 0, 0, 0) != 0) {
                throw std::invalid_argument(std::string("lua pcall error - ") + lua_tostring(m_state, -1));
            }
#else
            lua_call(m_state, 0, 0);
#endif
        }
        auto& asfunc = m_coroutines[Callback::OnStart];
        if (autoplay && asfunc.thread) {
            int retvalcount = 1;
            if (lua_resume(asfunc.thread, 0) == LUA_YIELD) {
                asfunc.resumable = true;
            } else {
                asfunc.resumable = false;
            }
            if (retvalcount > 0) {
                asfunc.delay = static_cast<float>(lua_tonumber(asfunc.thread, -1));
            }
        }
    } else {
#ifdef DEBUG
        throw std::invalid_argument(std::string("lua parse error - ") + lua_tostring(m_state, -1));
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
        if (lua_pcall(m_state, 1, 0, 0) != 0) {
            throw std::invalid_argument(std::string("lua pcall error - ") + lua_tostring(m_state, -1));
        }
#else
        lua_call(m_state, 1, 0);
#endif
    }
    auto& asfunc = m_coroutines[Callback::OnUpdate];
    if (asfunc.thread && asfunc.delay <= 0) {
        lua_pushnumber(asfunc.thread, deltatime);
        int retvalcount = 1;
        if (lua_resume(asfunc.thread, 1) == LUA_YIELD) {
            asfunc.resumable = true;
        } else {
            asfunc.resumable = false;
        }
        if (retvalcount > 0) {
            asfunc.delay = static_cast<float>(lua_tonumber(asfunc.thread, -1));
        }
    }
    for (auto& [_, co] : m_coroutines) {
        co.delay -= deltatime;
        if (co.delay <= 0 && co.resumable) {
            int retvalcount = 1;
            if (lua_resume(co.thread, 0) == LUA_YIELD) {
                co.resumable = true;
            } else {
                co.resumable = false;
            }
            if (retvalcount > 0) {
                co.delay = static_cast<float>(lua_tonumber(co.thread, -1));
            }
        }
    }
}

