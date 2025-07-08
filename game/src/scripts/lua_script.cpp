#include "lua/lua.h"
#include "pch.h"
#include "lua_script.h"
#include "lua_init.h"
#include "objects/entity.h"
#include "time/deltatime.h"
#include "world/area.h"


LuaScript::LuaScript(Area& parent_area) : p_parent_area(&parent_area), m_state(p_parent_area->lua_vm) {}

LuaScript::LuaScript(Area& parent_area, const std::string& filename)
    : p_parent_area(&parent_area), m_state(p_parent_area->lua_vm)
{
    load_from_file(filename);
}


static bool lua_hasfunction(lua_State* L, const char* name) {
    lua_pushstring(L, name);
    lua_rawget(L, -2);
    if (lua_isfunction(L, -1)) {
        lua_pop(L, 1);
        return true;
    } else {
        lua_pop(L, 1);
        return false;
    }
}

static void check_permutations(uint32_t funcs, LuaScript::AsyncCallback* coroutines) {
    if (!(funcs & (1 << (uint32_t)LuaScript::Callback::OnStart)) && !coroutines[(size_t)LuaScript::Callback::OnStart].resumable &&
        !(funcs & (1 << (uint32_t)LuaScript::Callback::OnUpdate)) && !coroutines[(size_t)LuaScript::Callback::OnUpdate].resumable &&
        !(funcs & (1 << (uint32_t)LuaScript::Callback::OnCreate)) &&
        !(funcs & (1 << (uint32_t)LuaScript::Callback::OnExit))) {
        std::cerr << "lua parse error - script does not define any event handlers\n";
        exit(1);
    }
    if ((funcs & (1 << (uint32_t)LuaScript::Callback::OnStart)) && coroutines[(size_t)LuaScript::Callback::OnStart].resumable) {
        std::cerr << "lua parse error - 'OnStart' event handler is multiply defined\n";
        exit(1);
    }
    if ((funcs & (1 << (uint32_t)LuaScript::Callback::OnUpdate)) && coroutines[(size_t)LuaScript::Callback::OnUpdate].resumable) {
        std::cerr << "lua parse error - 'OnUpdate' event handler is multiply defined\n";
        exit(1);
    }
}


static int block_globals(lua_State* L) {
    const char* key = lua_tostring(L, 2);
    luaL_error(L, "attempted to access unsafe or nonexistent global: %s", key);
    return 0;
}

void LuaScript::load_from_file(const std::string& filename) {
#ifdef DEBUG
    if (luaL_loadfile(m_state, filename.c_str()) != LUA_OK) {
        std::cerr << "lua parse error - " << lua_tostring(m_state, -1) << "\n";
        exit(1);
    }
#else
    luaL_loadfile(m_state, filename.c_str());
#endif

    lua_newtable(m_state); // chunk, env
    lua_getglobal(m_state, "shmy");
    lua_setfield(m_state, -2, "shmy");
    lua_getglobal(m_state, "math");
    lua_setfield(m_state, -2, "math");
    lua_getglobal(m_state, "string");
    lua_setfield(m_state, -2, "string");
    lua_getglobal(m_state, "table");
    lua_setfield(m_state, -2, "table");
    lua_getglobal(m_state, "coroutine");
    lua_setfield(m_state, -2, "coroutine");

    lua_getglobal(m_state, "print");
    lua_setfield(m_state, -2, "print");
    lua_getglobal(m_state, "pcall");
    lua_setfield(m_state, -2, "pcall");
    lua_getglobal(m_state, "pairs");
    lua_setfield(m_state, -2, "pairs");
    lua_getglobal(m_state, "ipairs");
    lua_setfield(m_state, -2, "ipairs");
    lua_getglobal(m_state, "tonumber");
    lua_setfield(m_state, -2, "tonumber");
    lua_getglobal(m_state, "tostring");
    lua_setfield(m_state, -2, "tostring");
    lua_getglobal(m_state, "next");
    lua_setfield(m_state, -2, "next");
    lua_getglobal(m_state, "select");
    lua_setfield(m_state, -2, "select");
    lua_getglobal(m_state, "unpack");
    lua_setfield(m_state, -2, "unpack");

    lua_newtable(m_state); // chunk, env, meta
    lua_pushcfunction(m_state, block_globals);
    lua_setfield(m_state, -2, "__index"); // chunk, env, meta
    lua_setmetatable(m_state, -2);

    lua_pushvalue(m_state, -1); // chunk, env, env
    lua_setfenv(m_state, -3);   // chunk, env

    m_env = luaL_ref(m_state, LUA_REGISTRYINDEX);
    lua_pushvalue(m_state, -1); // chunk, chunk
    m_chunk = luaL_ref(m_state, LUA_REGISTRYINDEX);

    if (lua_pcall(m_state, 0, 0, 0) != LUA_OK) {
        std::cerr << "lua pcall error - " << lua_tostring(m_state, -1) << "\n";
        exit(1);
    }

    lua_rawgeti(m_state, LUA_REGISTRYINDEX, m_env);
    if (lua_hasfunction(m_state, "OnUpdateAsync")) {
        luaL_loadstring(m_state, "function OnUpdateAsyncImpl(deltatime) while true do OnUpdateAsync(deltatime) deltatime = shmy.yield() end end");
        lua_rawgeti(m_state, LUA_REGISTRYINDEX, m_env);
        lua_setfenv(m_state, -2);
        if (lua_pcall(m_state, 0, 0, 0) != LUA_OK) {
            std::cerr << "lua pcall error - " << lua_tostring(m_state, -1) << "\n";
            exit(1);
        }
    }

    lua_pushlightuserdata(m_state, this);
    lua_setfield(m_state, LUA_REGISTRYINDEX, "script");

    lua_set_handlers(m_state, m_funcs);
    lua_load_coroutines(m_state, m_coroutines);
#ifdef DEBUG
    check_permutations(m_funcs, m_coroutines);
#endif

    if ((m_funcs & (1 << (uint32_t)Callback::OnCreate))) {
        lua_getfield(m_state, -1, "OnCreate");
#ifdef DEBUG
        if (lua_pcall(m_state, 0, 0, 0) != LUA_OK) {
            std::cerr << "lua pcall error - " << lua_tostring(m_state, -1) << "\n";
            exit(1);
        }
#else
        lua_call(m_state, 0, 0);
#endif
    }
    lua_pop(m_state, 1);
}


Entity& LuaScript::lookup_entity(const std::string& id) {
    return p_parent_area->entities.at(p_parent_area->script_name_LUT.at(id));
}

const Entity& LuaScript::lookup_entity(const std::string& id) const {
    return p_parent_area->entities.at(p_parent_area->script_name_LUT.at(id));
}



void LuaScript::start() {
    if ((m_funcs & (1 << (uint32_t)Callback::OnStart))) {
        lua_rawgeti(m_state, LUA_REGISTRYINDEX, m_env);
        lua_getfield(m_state, -1, "OnStart");
#ifdef DEBUG
        if (lua_pcall(m_state, 0, 0, 0) != LUA_OK) {
            std::cerr << "lua pcall error - " << lua_tostring(m_state, -1) << "\n";
            exit(1);
        }
#else
        lua_call(m_state, 0, 0);
#endif
        lua_pop(m_state, 1);
    }

    auto& async_start = m_coroutines[(size_t)Callback::OnStart];
    if (async_start.resumable) {
        switch (lua_resume(async_start.thread, 0)) {
        case LUA_YIELD:
            if (const auto nresults = lua_gettop(async_start.thread); nresults == 1) {
                async_start.resumable = true;
                async_start.delay = static_cast<float>(lua_tonumber(async_start.thread, -1));
                lua_pop(async_start.thread, nresults);
#ifdef DEBUG
            } else {
                std::cerr << "lua coroutine error - (internal) coroutine.yield nresults != 1\n";
                exit(1);
#endif
            }
            break;
        case LUA_OK:
            async_start.thread = nullptr;
            async_start.resumable = false;
            break;
#ifdef DEBUG
        case LUA_ERRRUN:
            std::cerr << "lua coroutine error - " << lua_tostring(async_start.thread, -1) << "\n";
            exit(1);
#endif
        }
    }
}

void LuaScript::terminate() {
    if ((m_funcs & (1 << (uint32_t)Callback::OnExit))) {
        lua_rawgeti(m_state, LUA_REGISTRYINDEX, m_env);
        lua_getfield(m_state, -1, "OnExit");
#ifdef DEBUG
        if (lua_pcall(m_state, 0, 0, 0) != LUA_OK) {
            std::cerr << "lua pcall error - " << lua_tostring(m_state, -1) << "\n";
            exit(1);
        }
#else
        lua_call(m_state, 0, 0);
#endif
        lua_pop(m_state, 1);
    }
    m_state = nullptr;
}

void LuaScript::mark_for_termination() {
    m_terminate = true;
}

void LuaScript::update() {
    if (!m_state) { return; }
    if (m_terminate) {
        terminate();
        return;
    }

    const auto deltatime = Time::deltatime();

    if (m_funcs & (1 << (uint32_t)Callback::OnUpdate)) {
        lua_rawgeti(m_state, LUA_REGISTRYINDEX, m_env);
        lua_getfield(m_state, -1, "OnUpdate");
        lua_pushnumber(m_state, deltatime);
#ifdef DEBUG
        if (lua_pcall(m_state, 1, 0, 0) != LUA_OK) {
            std::cerr << "lua pcall error - " << lua_tostring(m_state, -1) << "\n";
            exit(1);
        }
#else
        lua_call(m_state, 1, 0);
#endif
        lua_pop(m_state, 1);
    }

    auto& async_start = m_coroutines[(size_t)Callback::OnStart];
    if (async_start.resumable) {
        async_start.delay -= deltatime;
        if (async_start.delay <= 0) {
            switch (lua_resume(async_start.thread, 0)) {
            case LUA_YIELD:
                if (const auto nresults = lua_gettop(async_start.thread); nresults == 1) {
                    async_start.resumable = true;
                    async_start.delay = static_cast<float>(lua_tonumber(async_start.thread, -1));
                    lua_pop(async_start.thread, nresults);
#ifdef DEBUG
                } else {
                    std::cerr << "lua coroutine error - (internal) coroutine.yield nresults != 1\n";
                    exit(1);
#endif
                }
                break;
            case LUA_OK:
                async_start.thread = nullptr;
                async_start.resumable = false;
                break;
#ifdef DEBUG
            case LUA_ERRRUN:
                std::cerr << "lua coroutine error - " << lua_tostring(async_start.thread, -1) << "\n";
                exit(1);
#endif
            }
        }
    }

    auto& async_update = m_coroutines[(size_t)Callback::OnUpdate];
    if (async_update.resumable) {
        async_update.delay -= deltatime;
        if (async_update.delay <= 0) {
            lua_pushnumber(async_update.thread, deltatime);
            switch (lua_resume(async_update.thread, 1)) {
            case LUA_YIELD: {
                if (const auto nresults = lua_gettop(async_update.thread); nresults == 1) {
                    async_update.resumable = true;
                    async_update.delay = static_cast<float>(lua_tonumber(async_update.thread, -1));
                    lua_pop(async_update.thread, nresults);
#ifdef DEBUG
                } else {
                    std::cerr << "lua coroutine error - (internal) coroutine.yield nresults != 1\n";
                    exit(1);
#endif
                }
                break; }
            case LUA_OK:
                async_update.thread = nullptr;
                async_update.resumable = false;
                break;
#ifdef DEBUG
            case LUA_ERRRUN:
                std::cerr << "lua coroutine error - " << lua_tostring(async_update.thread, -1) << "\n";
                exit(1);
#endif
            }
        }
    }
}

