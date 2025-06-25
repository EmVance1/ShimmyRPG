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

static void lua_initcoroutine(lua_State* L, const char* name, LuaScript::AsyncCallback& as) {
    as.thread = lua_newthread(L);
    lua_getglobal(as.thread, name);
    lua_pop(L, -1);
}


void lua_register_engine_funcs(lua_State* L) {
    lua_register(L, "set_flag",       &l_set_flag);
    lua_register(L, "get_flag",       &l_get_flag);
    lua_register(L, "lock",           &l_lock_entity);
    lua_register(L, "unlock",         &l_unlock_entity);
    lua_register(L, "set_mode",       &l_set_mode);
    lua_register(L, "set_path",       &l_set_path);
    lua_register(L, "set_position",   &l_set_position);
    lua_register(L, "get_position",   &l_get_position);
    lua_register(L, "set_animation",  &l_set_animation);
    lua_register(L, "set_voicebank",  &l_set_voicebank);
    lua_register(L, "start_dialogue", &l_start_dialogue);
    lua_register(L, "set_camera",     &l_set_camera);
    lua_register(L, "set_combat",     &l_set_combat);
}

void lua_set_overriden_funcs(lua_State* L, uint32_t& funcs) {
    if (lua_hasfunction(L, "OnStart")) {
        funcs |= (uint32_t)LuaScript::Callback::OnStart;
    }
    if (lua_hasfunction(L, "OnUpdate")) {
        funcs |= (uint32_t)LuaScript::Callback::OnUpdate;
    }
}

void lua_set_coroutines(lua_State* L, std::unordered_map<LuaScript::Callback, LuaScript::AsyncCallback>& coroutines) {
    if (lua_hasfunction(L, "OnStartAsync")) {
        lua_initcoroutine(L, "OnStartAsync", coroutines[LuaScript::Callback::OnStart]);
    }
    if (lua_hasfunction(L, "OnUpdateAsync")) {
        lua_initcoroutine(L, "OnUpdateAsync", coroutines[LuaScript::Callback::OnUpdate]);
    }
}

