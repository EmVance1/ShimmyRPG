#include "pch.h"
#include <unordered_set>
#include "util/env.h"
#include "scripting/speech/graph.h"
#include "scripting/lua/script.h"
#include "scripting/lua/conversions.h"
#include "flags.h"
#include "world/area.h"
#include "world/region.h"
#include "objects/entity.h"


static int l_entity_get(lua_State* L);
static int l_set_overlay(lua_State* L);
static int l_goto_area(lua_State* L);
static int l_exit(lua_State* L);
static int l_set_mode(lua_State* L);
static int l_yield(lua_State* L);
static int l_yield_seconds(lua_State* L);
static int l_yield_combat(lua_State* L);
static int l_yield_dialogue(lua_State* L);
static int l_yield_exit(lua_State* L);
static int l_camera_set_pos(lua_State* L);
static int l_camera_set_target(lua_State* L);
static int l_camera_set_zoom(lua_State* L);
static int l_set_flag(lua_State* L);
static int l_get_flag(lua_State* L);
static int l_set_or_create_flag(lua_State* L);
static int l_magic_flag_get(lua_State* L);
static int l_magic_flag_set(lua_State* L);


void init_engine_api(lua_State* L) {
    lua_createtable(L, 0, 14);

    lua_pushcfunction(L, &l_entity_get);
    lua_setfield(L, -2, "entity");
    lua_pushcfunction(L, &l_set_overlay);
    lua_setfield(L, -2, "set_overlay");
    lua_pushcfunction(L, &l_goto_area);
    lua_setfield(L, -2, "goto_area");
    lua_pushcfunction(L, &l_exit);
    lua_setfield(L, -2, "exit");

    lua_pushcfunction(L, &l_set_mode);
    lua_setfield(L, -2, "set_mode");
    lua_pushcfunction(L, &l_yield);
    lua_setfield(L, -2, "yield");
    lua_pushcfunction(L, &l_yield_seconds);
    lua_setfield(L, -2, "yield_seconds");
    lua_pushcfunction(L, &l_yield_combat);
    lua_setfield(L, -2, "yield_to_combat");
    lua_pushcfunction(L, &l_yield_dialogue);
    lua_setfield(L, -2, "yield_to_dialogue");
    lua_pushcfunction(L, &l_yield_exit);
    lua_setfield(L, -2, "yield_exit");

    lua_pushstring(L, "camera");
    lua_createtable(L, 0, 3);
    lua_pushcfunction(L, &l_camera_set_pos);
    lua_setfield(L, -2, "set_position");
    lua_pushcfunction(L, &l_camera_set_target);
    lua_setfield(L, -2, "set_target");
    lua_pushcfunction(L, &l_camera_set_zoom);
    lua_setfield(L, -2, "set_zoom");
    lua_settable(L, -3);

    lua_pushstring(L, "flags");
    lua_newtable(L);
    lua_pushcfunction(L, &l_set_flag);
    lua_setfield(L, -2, "set");
    lua_pushcfunction(L, &l_get_flag);
    lua_setfield(L, -2, "get");
    lua_pushcfunction(L, &l_set_or_create_flag);
    lua_setfield(L, -2, "create");

    lua_newtable(L);
    lua_pushcfunction(L, &l_magic_flag_get);
    lua_setfield(L, -2, "__index");
    lua_pushcfunction(L, &l_magic_flag_set);
    lua_setfield(L, -2, "__newindex");
    lua_setmetatable(L, -2);
    lua_settable(L, -3);

    lua_pushstring(L, "mode");
    lua_createtable(L, 0, 3);
    lua_pushinteger(L, 0);
    lua_setfield(L, -2, "NORMAL");
    lua_pushinteger(L, 1);
    lua_setfield(L, -2, "CUTSCENE");
    lua_pushinteger(L, 2);
    lua_setfield(L, -2, "CINEMATIC");
    lua_pushinteger(L, 3);
    lua_setfield(L, -2, "COMBAT");
    lua_settable(L, -3);

    lua_setglobal(L, "shmy");
}


static void create_entity_table(lua_State* L, Entity* e);


static int l_entity_get(lua_State* L) {
    const auto entity = lua_tostring(L, 1);
    lua_getfield(L, LUA_REGISTRYINDEX, "_area");
    const auto area = static_cast<Area*>(lua_touserdata(L, -1));
    create_entity_table(L, &area->entities.at(area->script_to_uuid.at(entity)));

    return 1;
}

static int l_entity_set_offstage(lua_State* L) {
    lua_pushstring(L, "ptr");
    lua_gettable(L, 1);
    Entity* entity = (Entity*)lua_touserdata(L, -1);
    entity->set_offstage(lua_toboolean(L, 2));

    return 0;
}

static int l_entity_set_path(lua_State* L) {
    lua_pushstring(L, "ptr");
    lua_gettable(L, 1);
    Entity* entity = (Entity*)lua_touserdata(L, -1);
    const auto vec = lua_tovec2f(L, 2);
    entity->get_tracker().set_target_position({ vec.x, vec.y });

    return 0;
}

static int l_entity_set_position(lua_State* L) {
    lua_pushstring(L, "ptr");
    lua_gettable(L, 1);
    const auto entity = (Entity*)lua_touserdata(L, -1);
    const auto pos = lua_tovec2f(L, 2);
    lua_getfield(L, LUA_REGISTRYINDEX, "_area");
    const auto area = static_cast<Area*>(lua_touserdata(L, -1));
    entity->set_position(pos, area->cart_to_iso);

    return 0;
}

static int l_entity_get_position(lua_State* L) {
    lua_pushstring(L, "ptr");
    lua_gettable(L, 1);
    const Entity* entity = (Entity*)lua_touserdata(L, -1);
    const auto vec = entity->get_tracker().get_position();
    lua_pushvec2f(L, { vec.x, vec.y });

    return 1;
}

static int l_entity_set_animation(lua_State* L) {
    lua_pushstring(L, "ptr");
    lua_gettable(L, 1);
    const auto entity = (Entity*)lua_touserdata(L, -1);
    entity->set_animation(lua_tointeger(L, 2));
    return 0;
}

static int l_entity_set_voicebank(lua_State* L) {
    lua_pushstring(L, "ptr");
    lua_gettable(L, 1);
    const auto entity = (Entity*)lua_touserdata(L, -1);
    const auto voicebank = lua_tointeger(L, 2);
    (void)entity; (void)voicebank;
    // entity->set_voicebank(voicebank);
    //
    return 0;
}

static int l_entity_set_locked(lua_State* L) {
    lua_pushstring(L, "ptr");
    lua_gettable(L, 1);
    const auto entity = (Entity*)lua_touserdata(L, -1);
    const auto lock = lua_toboolean(L, 2);
    if (lock) {
        entity->get_tracker().stop();
    } else {
        entity->get_tracker().start();
    }
    return 0;
}

static int l_entity_set_paused(lua_State* L) {
    lua_pushstring(L, "ptr");
    lua_gettable(L, 1);
    const auto entity = (Entity*)lua_touserdata(L, -1);
    const auto lock = lua_toboolean(L, 2);
    if (lock) {
        entity->get_tracker().pause();
    } else {
        entity->get_tracker().start();
    }
    return 0;
}


static int l_camera_set_pos(lua_State* L) {
    const auto pos = lua_tovec2f(L, 1);
    lua_getfield(L, LUA_REGISTRYINDEX, "_area");
    const auto area = static_cast<Area*>(lua_touserdata(L, -1));
    area->camera.setCenter(area->cart_to_iso.transformPoint(pos));

    return 0;
}

static int l_camera_set_target(lua_State* L) {
    const auto pos = lua_tovec2f(L, 1);
    lua_getfield(L, LUA_REGISTRYINDEX, "_area");
    const auto area = static_cast<Area*>(lua_touserdata(L, -1));
    area->camera.setTrackingPos(area->cart_to_iso.transformPoint(pos));

    return 0;
}

static int l_camera_set_zoom(lua_State* L) {
    const auto scale = (float)lua_tonumber(L, 1);
    lua_getfield(L, LUA_REGISTRYINDEX, "_area");
    const auto area = static_cast<Area*>(lua_touserdata(L, -1));
    area->zoom_target = scale;

    return 0;
}


static int l_set_flag(lua_State* L) {
    const auto flag = lua_tostring(L, 1);
    if (lua_isboolean(L, 2)) {
        FlagTable::set_flag(flag, (uint32_t)lua_toboolean(L, 2), true);
    } else {
        FlagTable::set_flag(flag, (uint32_t)lua_tointeger(L, 2), true);
    }

    return 0;
}

static int l_get_flag(lua_State* L) {
    const auto flag = lua_tostring(L, 1);
    const auto value = FlagTable::get_flag(flag, true);
    lua_pushinteger(L, value);

    return 1;
}

static int l_set_or_create_flag(lua_State* L) {
    const auto flag = lua_tostring(L, 1);
    if (lua_isboolean(L, 2)) {
        FlagTable::set_flag(flag, (uint32_t)lua_toboolean(L, 2), false);
    } else {
        FlagTable::set_flag(flag, (uint32_t)lua_tointeger(L, 2), false);
    }

    return 0;
}


static int l_magic_flag_set(lua_State* L) {
    const auto flag = lua_tostring(L, 2);
    if (lua_isboolean(L, 3)) {
        FlagTable::set_flag(flag, (uint32_t)lua_toboolean(L, 3), true);
    } else {
        FlagTable::set_flag(flag, (uint32_t)lua_tointeger(L, 3), true);
    }

    return 0;
}

static int l_magic_flag_get(lua_State* L) {
    const auto flag = lua_tostring(L, 2);
    const auto value = FlagTable::get_flag(flag, true);
    lua_pushinteger(L, value);

    return 1;
}


static int l_set_mode(lua_State* L) {
    const auto mode = lua_tointeger(L, 1);
    lua_getfield(L, LUA_REGISTRYINDEX, "_area");
    const auto area = static_cast<Area*>(lua_touserdata(L, -1));
    switch (mode) {
    case 0: area->set_mode(GameMode::Normal); break;
    case 1: area->set_mode(GameMode::Dialogue); break;
    case 2: area->set_mode(GameMode::Cinematic); break;
    case 3: area->set_mode(GameMode::Combat); break;
    }

    return 0;
}

static int l_yield(lua_State* L) {
    if (lua_gettop(L) != 0) {
        return luaL_error(L, "yield() does not take any arguments");
    }
    lua_pushnumber(L, 0.f);
    return lua_yield(L, 1);
}

static int l_yield_seconds(lua_State* L) {
    if (lua_gettop(L) != 1) {
        return luaL_error(L, "wait_seconds(n) takes exactly one argument");
    }
    return lua_yield(L, 1);
}

static int l_yield_combat(lua_State* L) {
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

    lua_getfield(L, LUA_REGISTRYINDEX, "_area");
    const auto area = static_cast<Area*>(lua_touserdata(L, -1));
    area->begin_combat(ally_tags, enemy_tags, {}, {});

    return 0;
}

#ifdef VANGO_DEBUG
static int l_yield_dialogue(lua_State* L) {
    const auto filename = lua_tostring(L, 1);
    try {
        lua_getfield(L, LUA_REGISTRYINDEX, "_area");
        const auto area = static_cast<Area*>(lua_touserdata(L, -1));
        auto dia = shmy::speech::Graph::load_from_file(shmy::env::pkg_full() / filename);
        area->begin_dialogue(std::move(dia), filename);
    } catch (const std::exception& e) {
        std::cerr << "dialogue error: " << e.what() << "\n";
        exit(1);
    }

    lua_pushnumber(L, 0.0);
    return lua_yield(L, 1);
}
#else
static int l_yield_dialogue(lua_State* L) {
    const auto filename = lua_tostring(L, 1);
    lua_getfield(L, LUA_REGISTRYINDEX, "_area");
    const auto area = static_cast<Area*>(lua_touserdata(L, -1));
    area->begin_dialogue(shmy::speech::Graph::load_from_file(shmy::env::pkg_full() / filename), filename);

    lua_pushnumber(L, 0.0);
    return lua_yield(L, 1);
}
#endif

static int l_yield_exit(lua_State* L) {
    lua_getfield(L, LUA_REGISTRYINDEX, "_script");
    auto script = static_cast<shmy::lua::Script*>(lua_touserdata(L, -1));
    script->mark_for_termination();

    lua_pushnumber(L, 0.f);
    return lua_yield(L, 1);
}


static int l_set_overlay(lua_State* L) {
    const auto col = lua_tocolor(L, 1);
    lua_getfield(L, LUA_REGISTRYINDEX, "_area");
    const auto area = static_cast<Area*>(lua_touserdata(L, -1));
    area->render_settings->overlay = col;

    return 0;
}


static int l_goto_area(lua_State* L) {
    const auto idx = lua_tointeger(L, 1);
    const auto spawnpos = lua_tovec2f(L, 2);
    lua_getfield(L, LUA_REGISTRYINDEX, "_area");
    const auto area = static_cast<Area*>(lua_touserdata(L, -1));
    const auto& cti = area->cart_to_iso;
    area->region->set_active_area(idx);
    area->region->get_active_area().get_player().set_position(spawnpos, cti);
    area->region->get_active_area().suppress_portals = true;

    return 0;
}


static int l_exit(lua_State* L) {
    lua_getfield(L, LUA_REGISTRYINDEX, "_script");
    auto script = static_cast<shmy::lua::Script*>(lua_touserdata(L, -1));
    script->mark_for_termination();

    return 0;
}


static void create_entity_table(lua_State* L, Entity* e) {
    lua_createtable(L, 0, 7);

    lua_pushstring(L, "ptr");
    lua_pushlightuserdata(L, e);
    lua_settable(L, -3);

    lua_pushstring(L, "set_offstage");
    lua_pushcfunction(L, l_entity_set_offstage);
    lua_settable(L, -3);

    lua_pushstring(L, "set_path");
    lua_pushcfunction(L, l_entity_set_path);
    lua_settable(L, -3);

    lua_pushstring(L, "set_position");
    lua_pushcfunction(L, l_entity_set_position);
    lua_settable(L, -3);

    lua_pushstring(L, "get_position");
    lua_pushcfunction(L, l_entity_get_position);
    lua_settable(L, -3);

    lua_pushstring(L, "set_animation");
    lua_pushcfunction(L, l_entity_set_animation);
    lua_settable(L, -3);

    lua_pushstring(L, "set_voicebank");
    lua_pushcfunction(L, l_entity_set_voicebank);
    lua_settable(L, -3);

    lua_pushstring(L, "set_pathing_locked");
    lua_pushcfunction(L, l_entity_set_locked);
    lua_settable(L, -3);

    lua_pushstring(L, "set_pathing_paused");
    lua_pushcfunction(L, l_entity_set_paused);
    lua_settable(L, -3);
}

