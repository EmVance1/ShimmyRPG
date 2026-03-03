#include "pch.h"
#include "scripting/lua/conversions.h"
#include "world/game.h"
#include "world/scene.h"
#include "objects/entity.h"


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


static int l_add_action(lua_State* L) {
    auto entity = get_inner(L, 1);
    auto game = get_game(L, true);
    const auto handle = game->entity_handle(entity->id());
    auto& scene = game->active_scene();

    lua_pushstring(L, "_type");
    lua_gettable(L, 2);
    switch ((Entity::Action)lua_tointeger(L, -1)) {
    case Entity::Action::SetPath: {
        lua_pushinteger(L, 1);
        lua_gettable(L, 2);
        entity->push_action(shmy::sim::SetPathAction{ handle, lua_tovec2f(L, -1) });
        break; }
    case Entity::Action::MoveTo: {
        lua_pushinteger(L, 1);
        lua_gettable(L, 2);
        entity->push_action(shmy::sim::MoveToAction{ handle, game->entity_handle(lua_tostring(L, -1)) });
        break; }
    case Entity::Action::Speak: {
        lua_pushinteger(L, 1);
        lua_gettable(L, 2);
        entity->push_action(shmy::sim::SpeakAction{ handle, game->entity_handle(lua_tostring(L, -1)) });
        break; }
    case Entity::Action::Examine: {
        lua_pushinteger(L, 1);
        lua_gettable(L, 2);
        entity->push_action(shmy::sim::ExamineAction{ handle, game->entity_handle(lua_tostring(L, -1)) });
        break; }
    case Entity::Action::UsePortal: {
        lua_pushinteger(L, 1);
        lua_gettable(L, 2);
        entity->push_action(shmy::sim::UsePortalAction{ handle, &scene.portals.at(lua_tostring(L, -1)) });
        break; }
    }
    return 0;
}

template<int N>
static int l_tag_action(lua_State* L) {
    lua_pushvalue(L, 1);
    lua_pushinteger(L, N);
    lua_setfield(L, -2, "_type");
    return 1;
}


void create_action_table(lua_State* L) {
    lua_newtable(L);

    lua_pushcfunction(L, l_tag_action<(int)Entity::Action::SetPath>);
    lua_setfield(L, -2, "SetPath");
    lua_pushcfunction(L, l_tag_action<(int)Entity::Action::MoveTo>);
    lua_setfield(L, -2, "MoveTo");
    lua_pushcfunction(L, l_tag_action<(int)Entity::Action::Speak>);
    lua_setfield(L, -2, "Speak");
    lua_pushcfunction(L, l_tag_action<(int)Entity::Action::Examine>);
    lua_setfield(L, -2, "Examine");
    lua_pushcfunction(L, l_tag_action<(int)Entity::Action::UsePortal>);
    lua_setfield(L, -2, "UsePortal");
}

void create_queue_table(lua_State* L, Entity* e) {
    lua_newtable(L);

    lua_pushlightuserdata(L, e);
    lua_setfield(L, -2, "ptr");

    lua_pushcfunction(L, l_add_action);
    lua_setfield(L, -2, "add_action");
}

