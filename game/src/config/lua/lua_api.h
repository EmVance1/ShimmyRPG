#pragma once

struct lua_State;

void init_scene_api(lua_State* L);
void init_entity_api(lua_State* L);
void init_audio_api(lua_State* L);

