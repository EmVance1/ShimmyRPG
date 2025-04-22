#include "pch.h"
#include "lua_script.h"
#include "lua_init.h"
#include "lua_convert.h"


static bool lua_isvalid(lua_State* L, int r) {
    if (r != LUA_OK) {
        std::cout << lua_tostring(L, -1);
        return false;
    }
    return true;
}


void Script::callbacks(const sf::Event& event) {
    if (auto mbp = event.getIf<sf::Event::MouseButtonPressed>()) {
        if (m_funcs & (uint32_t)Callback::OnMouseClick) {
            lua_getglobal(m_state, "OnMouseClick");
            lua_pushvec2i(m_state, sf::Vector2i(mbp->position));
            lua_pushnumber(m_state, (int)mbp->button);
            lua_call(m_state, 2, 0);
        }
    } else if (auto mmv = event.getIf<sf::Event::MouseMoved>()) {
        if (m_funcs & (uint32_t)Callback::OnMouseMove) {
            lua_getglobal(m_state, "OnMouseMove");
            lua_pushvec2i(m_state, sf::Vector2i(mmv->position));
            lua_call(m_state, 1, 0);
        }
    } else if (auto mbr = event.getIf<sf::Event::MouseButtonReleased>()) {
        if (m_funcs & (uint32_t)Callback::OnMouseRelease) {
            lua_getglobal(m_state, "OnMouseRelease");
            lua_pushvec2i(m_state, sf::Vector2i(mbr->position));
            lua_pushnumber(m_state, (int)mbr->button);
            lua_call(m_state, 2, 0);
        }
    } else if (auto mws = event.getIf<sf::Event::MouseWheelScrolled>()) {
        if (m_funcs & (uint32_t)Callback::OnMouseScroll) {
            lua_getglobal(m_state, "OnMouseScroll");
            lua_pushvec2i(m_state, sf::Vector2i(mws->position));
            lua_pushnumber(m_state, (int)mws->delta);
            lua_call(m_state, 2, 0);
        }
    } else if (auto kp = event.getIf<sf::Event::KeyPressed>()) {
        if (m_funcs & (uint32_t)Callback::OnKeyPress) {
            lua_getglobal(m_state, "OnKeyPress");
            lua_pushnumber(m_state, (int)kp->code);
            lua_call(m_state, 1, 0);
        }
    } else if (auto kr = event.getIf<sf::Event::KeyReleased>()) {
        if (m_funcs & (uint32_t)Callback::OnKeyRelease) {
            lua_getglobal(m_state, "OnKeyRelease");
            lua_pushnumber(m_state, (int)kr->code);
            lua_call(m_state, 1, 0);
        }
    }
}

void Script::async_callbacks(const sf::Event& event) {
    if (auto mbp = event.getIf<sf::Event::MouseButtonPressed>()) {
        auto& asfunc = m_coroutines[Callback::OnMouseClick];
        if (asfunc.thread && asfunc.delay <= 0) {
            lua_getglobal(m_state, "OnMouseClickAsync");
            lua_pushvec2i(m_state, sf::Vector2i(mbp->position));
            lua_pushnumber(m_state, (int)mbp->button);
            int retvalcount = 1;
            if (lua_resume(m_state, asfunc.thread, 2, &retvalcount) == LUA_YIELD) {
                asfunc.resumable = true;
            } else {
                asfunc.resumable = false;
            }
            if (retvalcount > 0) { //!lua_isnil(asfunc.thread, -1))
                asfunc.delay = static_cast<float>(lua_tonumber(m_state, -1));
            }
        }
    } else if (auto mmv = event.getIf<sf::Event::MouseMoved>()) {
        auto& asfunc = m_coroutines[Callback::OnMouseMove];
        if (asfunc.thread && asfunc.delay <= 0) {
            lua_getglobal(m_state, "OnMouseMoveAsync");
            lua_pushvec2i(m_state, sf::Vector2i(mmv->position));
            int retvalcount = 1;
            if (lua_resume(m_state, asfunc.thread, 2, &retvalcount) == LUA_YIELD) {
                asfunc.resumable = true;
            } else {
                asfunc.resumable = false;
            }
            if (retvalcount > 0) { //!lua_isnil(asfunc.thread, -1))
                asfunc.delay = static_cast<float>(lua_tonumber(m_state, -1));
            }
        }
    } else if (auto mbr = event.getIf<sf::Event::MouseButtonReleased>()) {
        auto& asfunc = m_coroutines[Callback::OnMouseRelease];
        if (asfunc.thread && asfunc.delay <= 0) {
            lua_getglobal(m_state, "OnMouseReleaseAsync");
            lua_pushvec2i(m_state, sf::Vector2i(mbr->position));
            lua_pushnumber(m_state, (int)mbr->button);
            int retvalcount = 1;
            if (lua_resume(m_state, asfunc.thread, 2, &retvalcount) == LUA_YIELD) {
                asfunc.resumable = true;
            } else {
                asfunc.resumable = false;
            }
            if (retvalcount > 0) { //!lua_isnil(asfunc.thread, -1))
                asfunc.delay = static_cast<float>(lua_tonumber(m_state, -1));
            }
        }
    } else if (auto mws = event.getIf<sf::Event::MouseWheelScrolled>()) {
        auto& asfunc = m_coroutines[Callback::OnMouseScroll];
        if (asfunc.thread && asfunc.delay <= 0) {
            lua_getglobal(m_state, "OnMouseScrollAsync");
            lua_pushvec2i(m_state, sf::Vector2i(mws->position));
            lua_pushnumber(m_state, mws->delta);
            int retvalcount = 1;
            if (lua_resume(m_state, asfunc.thread, 2, &retvalcount) == LUA_YIELD) {
                asfunc.resumable = true;
            } else {
                asfunc.resumable = false;
            }
            if (retvalcount > 0) { //!lua_isnil(asfunc.thread, -1))
                asfunc.delay = static_cast<float>(lua_tonumber(m_state, -1));
            }
        }
    } else if (auto kp = event.getIf<sf::Event::KeyPressed>()) {
        auto& asfunc = m_coroutines[Callback::OnKeyPress];
        if (asfunc.thread && asfunc.delay <= 0) {
            lua_getglobal(m_state, "OnKeyPressAsync");
            lua_pushnumber(m_state, (int)kp->code);
            int retvalcount = 1;
            if (lua_resume(m_state, asfunc.thread, 2, &retvalcount) == LUA_YIELD) {
                asfunc.resumable = true;
            } else {
                asfunc.resumable = false;
            }
            if (retvalcount > 0) { //!lua_isnil(asfunc.thread, -1))
                asfunc.delay = static_cast<float>(lua_tonumber(m_state, -1));
            }
        }
    } else if (auto kr = event.getIf<sf::Event::KeyReleased>()) {
        auto& asfunc = m_coroutines[Callback::OnKeyRelease];
        if (asfunc.thread && asfunc.delay <= 0) {
            lua_getglobal(m_state, "OnKeyReleaseAsync");
            lua_pushnumber(m_state, (int)kr->code);
            int retvalcount = 1;
            if (lua_resume(m_state, asfunc.thread, 2, &retvalcount) == LUA_YIELD) {
                asfunc.resumable = true;
            } else {
                asfunc.resumable = false;
            }
            if (retvalcount > 0) { //!lua_isnil(asfunc.thread, -1))
                asfunc.delay = static_cast<float>(lua_tonumber(m_state, -1));
            }
        }
    }
}


Script::Script(const std::string& filename) {
    load_from_file(filename);

    if (m_funcs & (uint32_t)Callback::OnCreate) {
        lua_getglobal(m_state, "OnCreate");
        lua_call(m_state, 0, 0);
    }
}


void Script::load_from_file(const std::string& filename) {
    m_state = luaL_newstate();
    luaL_openlibs(m_state);

    if (lua_isvalid(m_state, luaL_dofile(m_state, filename.c_str()))) {
        lua_pushlightuserdata(m_state, this);
        lua_setglobal(m_state, "Scene");

        lua_setOverridenFuncs(m_state, m_funcs);
        lua_setCoroutines(m_state, m_coroutines);
    }
}


void Script::handle_event(const sf::Event& event) {
    callbacks(event);
    async_callbacks(event);
}

void Script::update(float deltatime) {
    if (m_funcs & (uint32_t)Callback::OnUpdate) {
        lua_getglobal(m_state, "OnUpdate");
        lua_pushnumber(m_state, deltatime);
        lua_call(m_state, 1, 0);
    }
    auto& asfunc = m_coroutines[Callback::OnUpdate];
    if (asfunc.thread && asfunc.delay <= 0) {
        lua_getglobal(m_state, "OnUpdateAsync");
        lua_pushnumber(m_state, deltatime);
        int retvalcount = 1;
        if (lua_resume(m_state, asfunc.thread, 1, &retvalcount) == LUA_YIELD) {
            asfunc.resumable = true;
        } else {
            asfunc.resumable = false;
        }
        if (retvalcount > 0) {
            asfunc.delay = static_cast<float>(lua_tonumber(m_state, -1));
        }
    }
    for (auto& [_, co] : m_coroutines) {
        co.delay -= deltatime;
        if (co.delay <= 0 && co.resumable) {
            int retvalcount = 1;
            if (lua_resume(m_state, co.thread, 0, &retvalcount) == LUA_YIELD) {
                co.resumable = true;
            } else {
                co.resumable = false;
            }
            if (retvalcount > 0) {
                co.delay = static_cast<float>(lua_tonumber(m_state, -1));
            }
        }
    }
}

