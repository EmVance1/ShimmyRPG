#include "pch.h"
#include "lua_init.h"


static bool lua_hasfunction(lua_State* L, const char* name) {
    lua_getglobal(L, name);

    if (lua_isfunction(L, -1)) {
        lua_pop(L, 1);
        return true;
    }

    lua_pop(L, 1);
    return false;
}

static void lua_initcoroutine(lua_State* L, const char* name, Script::AsyncCallback& as) {
    as.thread = lua_newthread(L);
    lua_getglobal(L, "OnMouseClickAsync");
    lua_xmove(L, as.thread, 1);
}


void lua_setOverridenFuncs(lua_State* L, uint32_t& funcs) {
    if (lua_hasfunction(L, "OnMouseClick")) {
        funcs |= (uint32_t)Script::Callback::OnMouseClick;
    }
    if (lua_hasfunction(L, "OnMouseMove")) {
        funcs |= (uint32_t)Script::Callback::OnMouseMove;
    }
    if (lua_hasfunction(L, "OnMouseRelease")) {
        funcs |= (uint32_t)Script::Callback::OnMouseRelease;
    }
    if (lua_hasfunction(L, "OnMouseScroll")) {
        funcs |= (uint32_t)Script::Callback::OnMouseScroll;
    }

    if (lua_hasfunction(L, "OnKeyPress")) {
        funcs |= (uint32_t)Script::Callback::OnKeyPress;
    }
    if (lua_hasfunction(L, "OnKeyRelease")) {
        funcs |= (uint32_t)Script::Callback::OnKeyRelease;
    }

    if (lua_hasfunction(L, "OnCreate")) {
        funcs |= (uint32_t)Script::Callback::OnCreate;
    }
    if (lua_hasfunction(L, "OnUpdate")) {
        funcs |= (uint32_t)Script::Callback::OnUpdate;
    }
}

void lua_setCoroutines(lua_State* L, std::unordered_map<Script::Callback, Script::AsyncCallback>& coroutines) {
    if (lua_hasfunction(L, "OnMouseClickAsync")) {
        lua_initcoroutine(L, "OnMouseClickAsync", coroutines[Script::Callback::OnMouseClick]);
    }
    if (lua_hasfunction(L, "OnMouseMoveAsync")) {
        lua_initcoroutine(L, "OnMouseMoveAsync", coroutines[Script::Callback::OnMouseMove]);
    }
    if (lua_hasfunction(L, "OnMouseReleaseAsync")) {
        lua_initcoroutine(L, "OnMouseReleaseAsync", coroutines[Script::Callback::OnMouseRelease]);
    }
    if (lua_hasfunction(L, "OnMouseScrollAsync")) {
        lua_initcoroutine(L, "OnMouseScrollAsync", coroutines[Script::Callback::OnMouseScroll]);
    }

    if (lua_hasfunction(L, "OnKeyPressAsync")) {
        lua_initcoroutine(L, "OnKeyPressAsync", coroutines[Script::Callback::OnKeyPress]);
    }
    if (lua_hasfunction(L, "OnKeyReleaseAsync")) {
        lua_initcoroutine(L, "OnKeyReleaseAsync", coroutines[Script::Callback::OnKeyRelease]);
    }

    if (lua_hasfunction(L, "OnCreateAsync")) {
        lua_initcoroutine(L, "OnCreateAsync", coroutines[Script::Callback::OnCreate]);
    }
    if (lua_hasfunction(L, "OnUpdateAsync")) {
        lua_initcoroutine(L, "OnUpdateAsync", coroutines[Script::Callback::OnUpdate]);
    }
}

