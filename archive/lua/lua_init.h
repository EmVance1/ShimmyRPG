#pragma once
#include <unordered_map>
#include <lua/lua.h>
#include <cstdint>
#include "lua_script.h"


void lua_setOverridenFuncs(lua_State* L, uint32_t& funcs);
void lua_setCoroutines(lua_State* L, std::unordered_map<Script::Callback, Script::AsyncCallback>& coroutines);

