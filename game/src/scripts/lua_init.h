#pragma once
#include <unordered_map>
#include <cstdint>
extern "C" {
#include <lua/lua.h>
}
#include "lua_script.h"


void lua_register_engine_funcs(lua_State* L);
void lua_set_overriden_funcs(lua_State* L, uint32_t& funcs);
void lua_set_coroutines(lua_State* L, std::unordered_map<LuaScript::Callback, LuaScript::AsyncCallback>& coroutines);

