#pragma once
extern "C" {
#include <lua/lua.h>
}

int l_set_flag(lua_State* L);
int l_get_flag(lua_State* L);
int l_lock_entity(lua_State* L);
int l_unlock_entity(lua_State* L);
int l_set_mode(lua_State* L);
int l_set_path(lua_State* L);
int l_set_position(lua_State* L);
int l_get_position(lua_State* L);
int l_set_animation(lua_State* L);
int l_set_voicebank(lua_State* L);
int l_start_dialogue(lua_State* L);
int l_set_camera(lua_State* L);
int l_set_combat(lua_State* L);

