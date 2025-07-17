#include "pch.h"
#include "init.h"
#include "callbacks.h"


static bool lua_hasfunction(lua_State* L, const char* name) {
    lua_pushstring(L, name);
    lua_rawget(L, -2);
    if (lua_isfunction(L, -1)) {
        lua_pop(L, 1);
        return true;
    } else {
        lua_pop(L, 1);
        return false;
    }
}


namespace lua {

void init_engine_api(lua_State* L) {
    lua_createtable(L, 0, 14);

    lua_pushcfunction(L, &l_entity_get);
    lua_setfield(L, -2, "entity");
    lua_pushcfunction(L, &l_set_overlay);
    lua_setfield(L, -2, "set_overlay");
    lua_pushcfunction(L, &l_goto_area);
    lua_setfield(L, -2, "goto_area");
    lua_pushcfunction(L, &l_exit);
    lua_setfield(L, -2, "exit");

    lua_pushcfunction(L, &l_set_mode);
    lua_setfield(L, -2, "set_mode");
    lua_pushcfunction(L, &l_yield);
    lua_setfield(L, -2, "yield");
    lua_pushcfunction(L, &l_yield_seconds);
    lua_setfield(L, -2, "yield_seconds");
    lua_pushcfunction(L, &l_yield_combat);
    lua_setfield(L, -2, "yield_to_combat");
    lua_pushcfunction(L, &l_yield_dialogue);
    lua_setfield(L, -2, "yield_to_dialogue");
    lua_pushcfunction(L, &l_yield_exit);
    lua_setfield(L, -2, "yield_exit");

    lua_pushstring(L, "camera");
    lua_createtable(L, 0, 3);
    lua_pushcfunction(L, &l_camera_set_pos);
    lua_setfield(L, -2, "set_position");
    lua_pushcfunction(L, &l_camera_set_target);
    lua_setfield(L, -2, "set_target");
    lua_pushcfunction(L, &l_camera_set_zoom);
    lua_setfield(L, -2, "set_zoom");
    lua_settable(L, -3);

    lua_pushstring(L, "flags");
    lua_newtable(L);
    lua_pushcfunction(L, &l_set_flag);
    lua_setfield(L, -2, "set");
    lua_pushcfunction(L, &l_get_flag);
    lua_setfield(L, -2, "get");
    lua_pushcfunction(L, &l_set_or_create_flag);
    lua_setfield(L, -2, "create");

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


void init_handlers(lua_State* L, uint32_t& funcs) {
    if (lua_hasfunction(L, "OnStart")) {
        funcs |= (1 << (uint32_t)Script::Callback::OnStart);
    }
    if (lua_hasfunction(L, "OnUpdate")) {
        funcs |= (1 << (uint32_t)Script::Callback::OnUpdate);
    }
    if (lua_hasfunction(L, "OnCreate")) {
        funcs |= (1 << (uint32_t)Script::Callback::OnCreate);
    }
    if (lua_hasfunction(L, "OnExit")) {
        funcs |= (1 << (uint32_t)Script::Callback::OnExit);
    }
}

void init_coroutines(lua_State* L, Script::AsyncCallback* coroutines, int env) {
    if (lua_hasfunction(L, "OnStartAsync")) {
        auto& cb = coroutines[(size_t)Script::Callback::OnStart];
        cb.thread = lua_newthread(L); // env, co
        cb.resumable = true;
        cb.delay = 0.f;
        lua_getfield(L, -2, "OnStartAsync"); // env, co, f   |
        lua_xmove(L, cb.thread, 1);          // env, co      | f
        lua_pop(L, 1);                       // env          | f
    }
    if (lua_hasfunction(L, "OnUpdateAsync")) {
        luaL_loadstring(L, "function OnUpdateAsyncImpl(deltatime) while true do OnUpdateAsync(deltatime) deltatime = shmy.yield() end end");
        lua_rawgeti(L, LUA_REGISTRYINDEX, env);
        lua_setfenv(L, -2);
        lua_call(L, 0, 0);

        auto& cb = coroutines[(size_t)Script::Callback::OnUpdate];
        cb.thread = lua_newthread(L);
        cb.resumable = true;
        cb.delay = 0.f;
        lua_getfield(L, -2, "OnUpdateAsyncImpl");
        lua_xmove(L, cb.thread, 1);
        lua_pop(L, 1);
    }
}

}
