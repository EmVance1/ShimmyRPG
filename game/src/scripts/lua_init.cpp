#include "pch.h"
#include "lua_init.h"
#include "lua_callbacks.h"


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


#define lua_registertable(L, name, func, idx) \
    lua_pushcfunction(L, func); \
    lua_setfield(L, idx, name)

void lua_register_engine_funcs(lua_State* L) {
    lua_createtable(L, 0, 14);

    lua_registertable(L, "entity",         &l_entity_get,         -2);
    lua_registertable(L, "set_flag",       &l_set_flag,           -2);
    lua_registertable(L, "get_flag",       &l_get_flag,           -2);
    lua_registertable(L, "create_flag",    &l_set_or_create_flag, -2);
    lua_registertable(L, "set_overlay",    &l_set_overlay,        -2);
    lua_registertable(L, "goto_area",      &l_goto_area,          -2);

    lua_registertable(L, "set_mode",          &l_set_mode,        -2);
    // lua_registertable(L, "set_combat",        &l_yield_combat,    -2);
    lua_registertable(L, "yield_to_combat",   &l_yield_combat,    -2);
    lua_registertable(L, "yield_to_dialogue", &l_yield_dialogue,  -2);
    lua_registertable(L, "yield_seconds",     &l_wait_seconds,    -2);
    lua_registertable(L, "yield",             &l_yield,           -2);

    lua_pushstring(L, "camera");
    lua_createtable(L, 0, 2);
    lua_registertable(L, "set_position", &l_camera_set_pos,    -2);
    lua_registertable(L, "set_target",   &l_camera_set_target, -2);
    lua_registertable(L, "zoom",         &l_camera_zoom,       -2);
    lua_settable(L, -3);

    lua_pushstring(L, "flags");
    lua_newtable(L);
    lua_newtable(L);
    lua_pushcfunction(L, &l_magic_flag_get);
    lua_setfield(L, -2, "__index");
    lua_pushcfunction(L, &l_magic_flag_set);
    lua_setfield(L, -2, "__newindex");
    lua_setmetatable(L, -2);
    lua_settable(L, -3);

    lua_pushstring(L, "mode");
    lua_createtable(L, 0, 3);
    lua_pushinteger(L, 0);
    lua_setfield(L, -2, "NORMAL");
    lua_pushinteger(L, 1);
    lua_setfield(L, -2, "CUTSCENE");
    lua_pushinteger(L, 2);
    lua_setfield(L, -2, "CINEMATIC");
    lua_pushinteger(L, 3);
    lua_setfield(L, -2, "COMBAT");
    lua_settable(L, -3);

    lua_setglobal(L, "shmy");
}


void lua_set_overriden_funcs(lua_State* L, uint32_t& funcs) {
    if (lua_hasfunction(L, "OnStart")) {
        funcs |= (1 << (uint32_t)LuaScript::Callback::OnStart);
    }
    if (lua_hasfunction(L, "OnUpdate")) {
        funcs |= (1 << (uint32_t)LuaScript::Callback::OnUpdate);
    }
}

void lua_set_coroutines(lua_State* L, LuaScript::AsyncCallback* coroutines) {
    if (lua_hasfunction(L, "OnStartAsync")) {
        auto& cb = coroutines[(size_t)LuaScript::Callback::OnStart];
        cb.thread = lua_newthread(L);
        cb.resumable = true;
        cb.delay = 0.f;
        lua_getglobal(cb.thread, "OnStartAsync");
        lua_pop(L, -1);
    }
    if (lua_hasfunction(L, "OnUpdateAsync")) {
        auto& cb = coroutines[(size_t)LuaScript::Callback::OnUpdate];
        cb.thread = lua_newthread(L);
        cb.resumable = true;
        cb.delay = 0.f;
        lua_getglobal(cb.thread, "OnUpdateAsyncImpl");
        lua_pop(L, -1);
    }
}

