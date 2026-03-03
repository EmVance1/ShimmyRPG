#include "pch.h"
#include "scripting/lua/conversions.h"
#include "data/flags.h"
#include "world/game.h"
#include "world/scene.h"


static int l_set_overlay(lua_State* L);
static int l_goto_scene(lua_State* L);
static int l_set_mode(lua_State* L);
static int l_yield(lua_State* L);
static int l_yield_seconds(lua_State* L);
static int l_yield_dialogue(lua_State* L);
static int l_camera_set_pos(lua_State* L);
static int l_camera_set_target(lua_State* L);
static int l_camera_set_zoom(lua_State* L);
static int l_set_flag(lua_State* L);
static int l_get_flag(lua_State* L);
// static int l_set_or_create_flag(lua_State* L);
static int l_magic_flag_get(lua_State* L);
static int l_magic_flag_set(lua_State* L);
void create_action_table(lua_State* L);


void init_scene_api(lua_State* L) {
    lua_newtable(L);

    lua_pushcfunction(L, &l_set_overlay);
    lua_setfield(L, -2, "set_overlay");
    lua_pushcfunction(L, &l_goto_scene);
    lua_setfield(L, -2, "goto_scene");

    lua_pushcfunction(L, &l_set_mode);
    lua_setfield(L, -2, "set_mode");
    lua_pushcfunction(L, &l_yield);
    lua_setfield(L, -2, "yield");
    lua_pushcfunction(L, &l_yield_seconds);
    lua_setfield(L, -2, "yield_seconds");
    lua_pushcfunction(L, &l_yield_dialogue);
    lua_setfield(L, -2, "yield_to_dialogue");

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
    // lua_pushcfunction(L, &l_set_or_create_flag);
    // lua_setfield(L, -2, "create");

    lua_newtable(L);
    lua_pushcfunction(L, &l_magic_flag_get);
    lua_setfield(L, -2, "__index");
    lua_pushcfunction(L, &l_magic_flag_set);
    lua_setfield(L, -2, "__newindex");
    lua_setmetatable(L, -2);
    lua_settable(L, -3);

    lua_newtable(L);
    lua_pushinteger(L, (int)Game::Mode::Simulation);
    lua_setfield(L, -2, "NORMAL");
    lua_pushinteger(L, (int)Game::Mode::Cinematic);
    lua_setfield(L, -2, "CINEMATIC");
    lua_setfield(L, -2, "mode");

    create_action_table(L);
    lua_setfield(L, -2, "action");

    lua_setglobal(L, "shmy");
}

static Game* get_game(lua_State* L, bool pop_stack = false) {
    lua_getfield(L, LUA_REGISTRYINDEX, "_game");
    auto game = static_cast<Game*>(lua_touserdata(L, -1));
    if (pop_stack) {
        lua_pop(L, 1);
    }
    return game;
}
static Scene* get_scene(lua_State* L, bool pop_stack = false) {
    return &get_game(L, pop_stack)->active_scene();
}


static int l_camera_set_pos(lua_State* L) {
    const auto pos = lua_tovec2f(L, 1);
    auto scene = get_scene(L);
    scene->camera.setCenter(scene->world_to_screen.transformPoint(pos));
    return 0;
}
static int l_camera_set_target(lua_State* L) {
    const auto pos = lua_tovec2f(L, 1);
    auto scene = get_scene(L);
    scene->camera.setTrackingPos(scene->world_to_screen.transformPoint(pos));
    return 0;
}
static int l_camera_set_zoom(lua_State* L) {
    const auto scale = (float)lua_tonumber(L, 1);
    get_scene(L)->camera.zoom(scale, sfu::Camera::ZoomFunc::Linear);
    return 0;
}

static int l_set_flag(lua_State* L) {
    const auto flag = lua_tostring(L, 1);
    if (lua_isboolean(L, 2)) {
        shmy::data::Flags::set(flag, (uint32_t)lua_toboolean(L, 2));
    } else {
        shmy::data::Flags::set(flag, (uint32_t)lua_tointeger(L, 2));
    }
    return 0;
}
static int l_get_flag(lua_State* L) {
    const auto flag = lua_tostring(L, 1);
    const auto value = shmy::data::Flags::get(flag);
    lua_pushinteger(L, (lua_Integer)value);
    return 1;
}
/*
static int l_set_or_create_flag(lua_State* L) {
    const auto flag = lua_tostring(L, 1);
    if (lua_isboolean(L, 2)) {
        shmy::data::Flags::set(flag, (uint32_t)lua_toboolean(L, 2), false);
    } else {
        shmy::data::Flags::set(flag, (uint32_t)lua_tointeger(L, 2), false);
    }
    return 0;
}
*/
static int l_magic_flag_set(lua_State* L) {
    const auto flag = lua_tostring(L, 2);
    if (lua_isboolean(L, 3)) {
        shmy::data::Flags::set(flag, (uint32_t)lua_toboolean(L, 3));
    } else {
        shmy::data::Flags::set(flag, (uint32_t)lua_tointeger(L, 3));
    }
    return 0;
}
static int l_magic_flag_get(lua_State* L) {
    const auto flag = lua_tostring(L, 2);
    const auto value = shmy::data::Flags::get(flag);
    lua_pushinteger(L, (lua_Integer)value);
    return 1;
}

static int l_set_mode(lua_State* L) {
    const auto nargs = lua_gettop(L);
    const auto mode = (Game::Mode)lua_tointeger(L, 1);
    auto game = get_game(L);
    if (nargs == 2) {
        game->set_mode(mode, true, lua_toboolean(L, 2));
    } else {
        game->set_mode(mode, true);
    }
    return 0;
}

static int l_yield(lua_State* L) {
    if (lua_gettop(L) != 0) {
        return luaL_error(L, "yield() does not take any arguments");
    }
    lua_pushnumber(L, 0.0);
    return lua_yield(L, 1);
}

static int l_yield_seconds(lua_State* L) {
    if (lua_gettop(L) != 1) {
        return luaL_error(L, "yield_seconds(n) takes exactly one argument");
    } else if (lua_tonumber(L, 1) < 0) {
        return luaL_error(L, "yield_seconds(n) argument must be positive");
    }
    return lua_yield(L, 1);
}

/*
static int l_yield_combat(lua_State* L) {
    auto tag_map = std::unordered_map<std::string, gamemode::Combat::Faction>();

    const size_t a_len = lua_objlen(L, 1);
    for (size_t i = 0; i < a_len; i++) {
        lua_pushinteger(L, i+1);
        lua_gettable(L, 1);
        tag_map[lua_tostring(L, -1)] = gamemode::Combat::Faction::Ally;
    }
    const size_t e_len = lua_objlen(L, 2);
    for (size_t i = 0; i < e_len; i++) {
        lua_pushinteger(L, i+1);
        lua_gettable(L, 2);
        tag_map[lua_tostring(L, -1)] = gamemode::Combat::Faction::Enemy;
    }

    get_scene(L)->game->combat_mode.begin(tag_map);
    lua_pushnumber(L, 0.0);
    return lua_yield(L, 1);
}
*/

#ifdef SHMY_DEBUG
static int l_yield_dialogue(lua_State* L) {
    const auto modpath = lua_tostring(L, 1);
    try {
        get_game(L)->cinematic_mode.signal_action(shmy::sim::Cinematic::BeginSpeech{ modpath });
    } catch (const std::exception& e) {
        std::cerr << "dialogue error: " << e.what() << "\n";
        exit(1);
    }

    lua_pushnumber(L, 0.0);
    return lua_yield(L, 1);
}
#else
static int l_yield_dialogue(lua_State* L) {
    const auto dia = lua_tostring(L, 1);
    get_game(L)->cinematic_mode.signal_action(shmy::sim::Cinematic::BeginSpeech{ dia, "" });

    lua_pushnumber(L, 0.0);
    return lua_yield(L, 1);
}
#endif


static int l_set_overlay(lua_State* L) {
    const auto col = lua_tocolor(L, 1);
    get_scene(L)->game->render_ctx.overlay = col;
    return 0;
}

static int l_goto_scene(lua_State* L) {
    const auto idx = (int)lua_tointeger(L, 1);
    const auto spawnpos = lua_tovec2f(L, 2);
    // get_scene(L)->game->queue_scene_swap(idx, UINT32_MAX, spawnpos);
    return 0;
}

