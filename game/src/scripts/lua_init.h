#pragma once
#include <cstdint>
extern "C" {
#include <lua/lua.h>
}
#include "lua_script.h"


void lua_register_engine_funcs(lua_State* L);
void lua_set_handlers(lua_State* L, uint32_t& funcs);
void lua_load_coroutines(lua_State* L, LuaScript::AsyncCallback* coroutines);

