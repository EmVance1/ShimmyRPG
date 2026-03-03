#include "pch.h"
#include "scripting/lua/conversions.h"
#include "world/game.h"
#include "world/scene.h"
#include "objects/entity.h"


static int l_get_entity(lua_State* L);

void init_entity_api(lua_State* L) {
    lua_getglobal(L, "shmy");
    lua_pushcfunction(L, &l_get_entity);
    lua_setfield(L, -2, "entity");
}


void create_entity_table(lua_State* L, Entity* e);
void create_queue_table(lua_State* L, Entity* e);


static Entity* get_inner(lua_State* L, int idx) {
    lua_pushstring(L, "ptr");
    lua_gettable(L, idx);
    return static_cast<Entity*>(lua_touserdata(L, -1));
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

static int l_get_entity(lua_State* L) {
    const auto entity = lua_tostring(L, 1);
    auto game = get_game(L, true);
    create_entity_table(L, &game->entity_by_id(entity));
    return 1;
}


static int l_set_offstage(lua_State* L) {
    auto entity = get_inner(L, 1);
    entity->set_offstage(lua_toboolean(L, 2));
    return 0;
}
static int l_set_path(lua_State* L) {
    auto entity = get_inner(L, 1);
    const auto vec = lua_tovec2f(L, 2);
    entity->get_tracker().set_target_position({ vec.x, vec.y });
    return 0;
}
static int l_set_position(lua_State* L) {
    auto entity = get_inner(L, 1);
    const auto pos = lua_tovec2f(L, 2);
    const auto scene = get_scene(L);
    entity->set_position(pos, scene->world_to_screen);
    return 0;
}
static int l_get_position(lua_State* L) {
    const auto entity = get_inner(L, 1);
    const auto vec = entity->get_tracker().get_position();
    lua_pushvec2f(L, { vec.x, vec.y });
    return 1;
}
static int l_set_animation(lua_State* L) {
    auto entity = get_inner(L, 1);
    entity->set_animation((size_t)lua_tointeger(L, 2));
    return 0;
}
static int l_set_voicebank(lua_State* L) {
    auto entity = get_inner(L, 1);
    const auto voicebank = lua_tointeger(L, 2);
    (void)entity; (void)voicebank;
    // entity->set_voicebank(voicebank);
    return 0;
}
static int l_set_locked(lua_State* L) {
    auto entity = get_inner(L, 1);
    if (lua_toboolean(L, 2)) {
        entity->get_tracker().stop();
    } else {
        entity->get_tracker().start();
    }
    return 0;
}
static int l_set_paused(lua_State* L) {
    auto entity = get_inner(L, 1);
    if (lua_toboolean(L, 2)) {
        entity->get_tracker().pause();
    } else {
        entity->get_tracker().start();
    }
    return 0;
}
static int l_use_portal(lua_State* L) {
    auto entity = get_inner(L, 1);
    auto game = get_game(L, true);
    const auto handle = game->entity_handle(entity->id());
    auto& scene = game->active_scene();
    const auto& portal = scene.portals[lua_tostring(L, 2)];
    game->normal_mode.signal_action(shmy::sim::UsePortalAction{ handle, &portal });

    return 0;
}
static int l_get_id(lua_State* L) {
    auto entity = get_inner(L, 1);
    lua_pushstring(L, entity->id().c_str());
    return 1;
}


void create_entity_table(lua_State* L, Entity* e) {
    lua_newtable(L);

    lua_pushlightuserdata(L, e);
    lua_setfield(L, -2, "ptr");

    lua_pushcfunction(L, l_set_offstage);
    lua_setfield(L, -2, "set_offstage");
    lua_pushcfunction(L, l_set_path);
    lua_setfield(L, -2, "set_path");
    lua_pushcfunction(L, l_set_position);
    lua_setfield(L, -2, "set_position");
    lua_pushcfunction(L, l_get_position);
    lua_setfield(L, -2, "get_position");
    lua_pushcfunction(L, l_set_animation);
    lua_setfield(L, -2, "set_animation");
    lua_pushcfunction(L, l_set_voicebank);
    lua_setfield(L, -2, "set_voicebank");
    lua_pushcfunction(L, l_set_locked);
    lua_setfield(L, -2, "set_pathing_locked");
    lua_pushcfunction(L, l_set_paused);
    lua_setfield(L, -2, "set_pathing_paused");
    lua_pushcfunction(L, l_use_portal);
    lua_setfield(L, -2, "use_portal");
    lua_pushcfunction(L, l_get_id);
    lua_setfield(L, -2, "get_id");

    create_queue_table(L, e);
    lua_setfield(L, -2, "queue");
}

