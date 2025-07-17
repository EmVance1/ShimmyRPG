#pragma once
#include <cstdint>
#include <lua/lua.hpp>
#include "script.h"


namespace lua {

void init_engine_api(lua_State* L);
void init_handlers(lua_State* L, uint32_t& funcs);
void init_coroutines(lua_State* L, Script::AsyncCallback* coroutines, int env);

}
