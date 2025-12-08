#include "pch.h"
#include "scripting/lua/runtime.h"


namespace shmy { namespace lua {

int l_register_handler(lua_State* L) {
    const auto event = lua_tostring(L, 1);
    lua_pushvalue(L, 2);
    const auto func  = luaL_ref(L, LUA_REGISTRYINDEX);

    lua_getfield(L, LUA_REGISTRYINDEX, "_locstate");
    const auto state = (int)lua_tointeger(L, -1);

    lua_getfield(L, LUA_REGISTRYINDEX, "_runtime");
    const auto runtime = (Runtime*)lua_touserdata(L, -1);
    runtime->register_handler(event, Runtime::Callback{ func, state });

    return 0;
}

int l_register_async_handler(lua_State* L) {
    const auto event = lua_tostring(L, 1);

    auto cb = Runtime::AsyncCallback{};

    cb.thread = lua_newthread(L);
    lua_getglobal(L, "_AsyncWrapper");
    lua_pushvalue(L, 2);
    lua_xmove(L, cb.thread, 2);
    lua_pop(L, 1);

    lua_resume(cb.thread, 1);

    lua_getfield(L, LUA_REGISTRYINDEX, "_locstate");
    cb.state = (int)lua_tointeger(L, -1);

    lua_getfield(L, LUA_REGISTRYINDEX, "_runtime");
    const auto runtime = (Runtime*)lua_touserdata(L, -1);
    runtime->register_async_handler(event, cb);

    return 0;
}

int l_dispatch_event(lua_State* L) {
    const auto event = lua_tostring(L, 1);
    lua_pushvalue(L, 2);
    const auto args = luaL_ref(L, LUA_REGISTRYINDEX);

    lua_getfield(L, LUA_REGISTRYINDEX, "_runtime");
    const auto runtime = (Runtime*)lua_touserdata(L, -1);
    runtime->on_event(event, EventArgs{ true, args });

    return 0;
}

} }

