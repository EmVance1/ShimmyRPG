#include "pch.h"
#include "events.h"


void create_entity_table(lua_State* L, Entity* e);

namespace event_arg {

shmy::lua::EventArgs none() {
    return shmy::lua::EventArgs{ false, -1 };
}

shmy::lua::EventArgs reached_dest(shmy::lua::Runtime& rt, Entity& e) {
    lua_State* L = rt.get_state();
    create_entity_table(L, &e);
    return shmy::lua::EventArgs{ true, luaL_ref(L, LUA_REGISTRYINDEX) };
}

}
