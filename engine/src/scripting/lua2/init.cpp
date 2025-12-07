#include "pch.h"
#include "scripting/lua2/runtime.h"


namespace shmy { namespace lua2 {

int l_register_handler(lua_State* L) {
    const auto event = lua_tostring(L, 1);
    lua_pushvalue(L, 2);
    const auto func  = luaL_ref(L, LUA_REGISTRYINDEX);

    lua_getfield(L, LUA_REGISTRYINDEX, "_runtime");
    const auto runtime = (Runtime*)lua_touserdata(L, -1);

    runtime->register_handler(event, func);

    return 0;
}

int l_register_async_handler(lua_State* L) {
    const auto event = lua_tostring(L, 1);
    lua_pushvalue(L, 2);
    const auto func  = luaL_ref(L, LUA_REGISTRYINDEX);

    lua_getfield(L, LUA_REGISTRYINDEX, "_runtime");
    const auto runtime = (Runtime*)lua_touserdata(L, -1);

    runtime->register_async_handler(event, func);

    return 0;
}

int l_dispatch_event(lua_State* L) {
    const auto event = lua_tostring(L, 1);
    lua_pushvalue(L, 2);
    const auto args = luaL_ref(L, LUA_REGISTRYINDEX);

    lua_getfield(L, LUA_REGISTRYINDEX, "_runtime");
    const auto runtime = (Runtime*)lua_touserdata(L, -1);

    runtime->on_event(event, args);

    luaL_unref(L, LUA_REGISTRYINDEX, args);

    return 0;
}

} }

