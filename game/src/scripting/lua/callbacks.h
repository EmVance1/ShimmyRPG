#pragma once
#include <lua/lua.hpp>


int l_entity_get(lua_State* L);
int l_entity_set_offstage(lua_State* L);
int l_entity_set_path(lua_State* L);
int l_entity_set_position(lua_State* L);
int l_entity_get_position(lua_State* L);
int l_entity_set_animation(lua_State* L);
int l_entity_set_voicebank(lua_State* L);
int l_entity_set_locked(lua_State* L);
int l_entity_set_paused(lua_State* L);

int l_camera_set_pos(lua_State* L);
int l_camera_set_target(lua_State* L);
int l_camera_set_zoom(lua_State* L);

int l_set_flag(lua_State* L);
int l_get_flag(lua_State* L);
int l_set_or_create_flag(lua_State* L);

int l_magic_flag_set(lua_State* L);
int l_magic_flag_get(lua_State* L);

int l_yield(lua_State* L);
int l_yield_seconds(lua_State* L);
int l_yield_combat(lua_State* L);
int l_yield_dialogue(lua_State* L);
int l_yield_exit(lua_State* L);

int l_set_mode(lua_State* L);
int l_set_overlay(lua_State* L);
int l_goto_area(lua_State* L);
int l_exit(lua_State* L);

