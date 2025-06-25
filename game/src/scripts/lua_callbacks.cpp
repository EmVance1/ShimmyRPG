#include "pch.h"
#include "lua_callbacks.h"
#include "lua_script.h"
#include "lua_convert.h"
#include "objects/entity.h"
#include "flags.h"
#include "scripting/speech_graph.h"
#include "world/area.h"
#include <unordered_set>


int l_set_flag(lua_State* L) {
    const auto flag = lua_tostring(L, 1);
    const auto value = (uint32_t)lua_tointeger(L, 2);
    FlagTable::set_flag(flag, value);

    return 0;
}

int l_get_flag(lua_State* L) {
    const auto flag = lua_tostring(L, 1);
    const auto value = FlagTable::get_flag(flag);
    lua_pushinteger(L, value);

    return 1;
}

int l_lock_entity(lua_State* L) {
    const auto entity = lua_tostring(L, 1);
    lua_pushstring(L, "script");
    lua_gettable(L, LUA_REGISTRYINDEX);
    const auto script = static_cast<LuaScript*>(lua_touserdata(L, -1));
    script->lookup_entity(entity).get_tracker().stop();

    return 0;
}

int l_unlock_entity(lua_State* L) {
    const auto entity = lua_tostring(L, 1);
    lua_pushstring(L, "script");
    lua_gettable(L, LUA_REGISTRYINDEX);
    const auto script = static_cast<LuaScript*>(lua_touserdata(L, -1));
    script->lookup_entity(entity).get_tracker().start();

    return 0;
}

int l_set_mode(lua_State* L) {
    const auto mode = lua_tointeger(L, 1);
    lua_pushstring(L, "script");
    lua_gettable(L, LUA_REGISTRYINDEX);
    const auto script = static_cast<LuaScript*>(lua_touserdata(L, -1));
    switch (mode) {
    case 0: script->parent_area().set_mode(GameMode::Normal); break;
    case 1: script->parent_area().set_mode(GameMode::Dialogue); break;
    case 2: script->parent_area().set_mode(GameMode::Cinematic); break;
    case 3: script->parent_area().set_mode(GameMode::Combat); break;
    }

    return 0;
}

int l_set_path(lua_State* L) {
    const auto entity = lua_tostring(L, 1);
    const auto target = lua_tovec2f(L, 2);
    lua_pushstring(L, "script");
    lua_gettable(L, LUA_REGISTRYINDEX);
    const auto script = static_cast<LuaScript*>(lua_touserdata(L, -1));
    script->lookup_entity(entity).get_tracker().set_target_position(target);

    return 0;
}

int l_set_position(lua_State* L) {
    const auto entity = lua_tostring(L, 1);
    const auto pos = lua_tovec2f(L, 2);
    lua_pushstring(L, "script");
    lua_gettable(L, LUA_REGISTRYINDEX);
    const auto script = static_cast<LuaScript*>(lua_touserdata(L, -1));
    script->lookup_entity(entity).set_position(pos, script->parent_area().cart_to_iso);

    return 0;
}

int l_get_position(lua_State* L) {
    const auto entity = lua_tostring(L, 1);
    lua_pushstring(L, "script");
    lua_gettable(L, LUA_REGISTRYINDEX);
    const auto script = static_cast<LuaScript*>(lua_touserdata(L, -1));
    const auto pos = script->lookup_entity(entity).get_tracker().get_position();
    lua_pushvec2f(L, pos);

    return 1;
}

int l_set_animation(lua_State* L) {
    const auto entity = lua_tostring(L, 1);
    const auto animation = lua_tointeger(L, 2);
    lua_pushstring(L, "script");
    lua_gettable(L, LUA_REGISTRYINDEX);
    const auto script = static_cast<LuaScript*>(lua_touserdata(L, -1));
    script->lookup_entity(entity).set_animation(animation);

    return 0;
}

int l_set_voicebank(lua_State* L) {
    const auto entity = lua_tostring(L, 1);
    const auto voicebank = lua_tointeger(L, 2);
    lua_pushstring(L, "script");
    lua_gettable(L, LUA_REGISTRYINDEX);
    const auto script = static_cast<LuaScript*>(lua_touserdata(L, -1));
    // script->lookup_entity(entity).set_voicebank(voicebank);

    return 0;
}

#ifdef DEBUG
int l_start_dialogue(lua_State* L) {
    const auto filename = lua_tostring(L, 1);
    try {
        lua_pushstring(L, "script");
        lua_gettable(L, LUA_REGISTRYINDEX);
        const auto script = static_cast<LuaScript*>(lua_touserdata(L, -1));
        script->parent_area().begin_dialogue(dialogue_from_file(filename), filename);
    } catch (const std::exception& e) {
        std::cout << e.what() << "\n";
    }

    return 0;
}
#else
int l_start_dialogue(lua_State* L) {
    const auto filename = lua_tostring(L, 1);
    lua_pushstring(L, "script");
    lua_gettable(L, LUA_REGISTRYINDEX);
    const auto script = static_cast<LuaScript*>(lua_touserdata(L, -1));
    script->parent_area().begin_dialogue(dialogue_from_file(filename), filename);

    return 0;
}
#endif

int l_set_camera(lua_State* L) {
    const auto pos = lua_tovec2f(L, 1);
    lua_pushstring(L, "script");
    lua_gettable(L, LUA_REGISTRYINDEX);
    const auto script = static_cast<LuaScript*>(lua_touserdata(L, -1));
    script->parent_area().camera.setTrackingPos(script->parent_area().cart_to_iso.transformPoint(pos));

    return 0;
}

int l_set_combat(lua_State* L) {
    const size_t a_len = lua_objlen(L, 1);
    auto ally_tags = std::unordered_set<std::string>();
    for (size_t i = 0; i < a_len; i++) {
        lua_pushinteger(L, i+1);
        lua_gettable(L, 1);
        ally_tags.insert(lua_tostring(L, -1));
    }
    const size_t e_len = lua_objlen(L, 2);
    auto enemy_tags = std::unordered_set<std::string>();
    for (size_t i = 0; i < e_len; i++) {
        lua_pushinteger(L, i+1);
        lua_gettable(L, 2);
        enemy_tags.insert(lua_tostring(L, -1));
    }

    lua_pushstring(L, "script");
    lua_gettable(L, LUA_REGISTRYINDEX);
    const auto script = static_cast<LuaScript*>(lua_touserdata(L, -1));
    script->parent_area().begin_combat(ally_tags, enemy_tags, {}, {});

    return 0;
}

