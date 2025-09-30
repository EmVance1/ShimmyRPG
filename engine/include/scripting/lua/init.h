#pragma once
#include <luajit-2.1/lua.hpp>
#include <cstdint>
#include "script.h"


namespace shmy { namespace lua {

void init_handlers(lua_State* L, uint32_t& funcs);
void init_coroutines(lua_State* L, Script::AsyncCallback* coroutines, int env);

} }
