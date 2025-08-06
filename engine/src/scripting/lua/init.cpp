#include "pch.h"
#include "init.h"


namespace shmy { namespace lua {

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

} }
