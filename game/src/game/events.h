#include "scripting/lua/runtime.h"
#include "objects/entity.h"


namespace event_arg {

shmy::lua::EventArgs none();
shmy::lua::EventArgs reached_dest(shmy::lua::Runtime& rt, Entity& e);

}

