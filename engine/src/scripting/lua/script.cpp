#include "pch.h"
#include "scripting/lua/script.h"
#include "scripting/lua/init.h"
#include "time/deltatime.h"
#include "util/str.h"


namespace shmy { namespace lua {

#ifdef VANGO_DEBUG
#define LUA_CHECK(f, pre) if (f != LUA_OK) { \
        std::cerr << pre << " - " << lua_tostring(p_L, -1) << "\n"; \
        exit(1); \
    }
#else
#define LUA_CHECK(f, pre) f
#endif


static void check_permutations(uint32_t funcs, Script::AsyncCallback* coroutines);
static int block_globals(lua_State* L);


Script::Script(lua_State* L, const std::fs::path& filename, const char* api) : p_L(L) {
    {
        const auto file = shmy::str::read_to_string(filename);
        LUA_CHECK(luaL_loadstring(p_L, file.c_str()), "lua parse error");
    }

    lua_newtable(p_L); // chunk, env
    lua_getglobal(p_L, api);
    lua_setfield(p_L, -2, api);
    lua_getglobal(p_L, "math");
    lua_setfield(p_L, -2, "math");
    lua_getglobal(p_L, "string");
    lua_setfield(p_L, -2, "string");
    lua_getglobal(p_L, "table");
    lua_setfield(p_L, -2, "table");
    lua_getglobal(p_L, "coroutine");
    lua_setfield(p_L, -2, "coroutine");

    lua_getglobal(p_L, "print");
    lua_setfield(p_L, -2, "print");
    lua_getglobal(p_L, "pcall");
    lua_setfield(p_L, -2, "pcall");
    lua_getglobal(p_L, "pairs");
    lua_setfield(p_L, -2, "pairs");
    lua_getglobal(p_L, "ipairs");
    lua_setfield(p_L, -2, "ipairs");
    lua_getglobal(p_L, "tonumber");
    lua_setfield(p_L, -2, "tonumber");
    lua_getglobal(p_L, "tostring");
    lua_setfield(p_L, -2, "tostring");
    lua_getglobal(p_L, "next");
    lua_setfield(p_L, -2, "next");
    lua_getglobal(p_L, "select");
    lua_setfield(p_L, -2, "select");
    lua_getglobal(p_L, "unpack");
    lua_setfield(p_L, -2, "unpack");

    lua_newtable(p_L); // chunk, env, meta
    lua_pushcfunction(p_L, block_globals);
    lua_setfield(p_L, -2, "__index"); // chunk, env, meta
    lua_setmetatable(p_L, -2);

    lua_pushvalue(p_L, -1); // chunk, env, env
    lua_setfenv(p_L, -3);   // chunk, env

    m_env = luaL_ref(p_L, LUA_REGISTRYINDEX);

    LUA_CHECK(lua_pcall(p_L, 0, 0, 0), "lua pcall error");

    lua_rawgeti(p_L, LUA_REGISTRYINDEX, m_env);

    init_handlers(p_L, m_funcs);
    init_coroutines(p_L, m_coroutines, m_env);
#ifdef VANGO_DEBUG
    check_permutations(m_funcs, m_coroutines);
#endif

    lua_pushlightuserdata(p_L, this);
    lua_setfield(p_L, LUA_REGISTRYINDEX, "_script");

    if ((m_funcs & (1 << (uint32_t)Callback::OnCreate))) {
        lua_getfield(p_L, -1, "OnCreate");
        LUA_CHECK(lua_pcall(p_L, 0, 0, 0), "lua pcall error");
    }
    lua_pop(p_L, 1);
}

Script::Script(Script&& other)
    : p_L(other.p_L),
    m_env(other.m_env),
    m_funcs(other.m_funcs),
    m_terminate(other.m_terminate)
{
    m_coroutines[0] = other.m_coroutines[0];
    m_coroutines[1] = other.m_coroutines[1];
    other.p_L = nullptr;
}

Script::~Script() {
    terminate();
}


void Script::start() {
    if (!p_L) { return; }
    if ((m_funcs & (1 << (uint32_t)Callback::OnStart))) {
        lua_rawgeti(p_L, LUA_REGISTRYINDEX, m_env);
        lua_getfield(p_L, -1, "OnStart");
        LUA_CHECK(lua_pcall(p_L, 0, 0, 0), "lua pcall error");
        lua_pop(p_L, 1);
    }

    auto& async_start = m_coroutines[(size_t)Callback::OnStart];
    if (async_start.resumable) {
        switch (lua_resume(async_start.thread, 0)) {
        case LUA_YIELD:
            if (const auto nresults = lua_gettop(async_start.thread); nresults == 1) {
                async_start.resumable = true;
                async_start.delay = static_cast<float>(lua_tonumber(async_start.thread, -1));
                lua_pop(async_start.thread, nresults);
            } else {
#ifdef VANGO_DEBUG
                std::cerr << "lua coroutine error - (internal) coroutine.yield nresults != 1\n";
                exit(1);
#endif
            }
            break;
        case LUA_OK:
            async_start.thread = nullptr;
            async_start.resumable = false;
            break;
#ifdef VANGO_DEBUG
        case LUA_ERRRUN:
            std::cerr << "lua coroutine error - " << lua_tostring(async_start.thread, -1) << "\n";
            exit(1);
#endif
        }
    }
}

void Script::terminate() {
    if (!p_L) { return; }
    if ((m_funcs & (1 << (uint32_t)Callback::OnExit))) {
        lua_rawgeti(p_L, LUA_REGISTRYINDEX, m_env);
        lua_getfield(p_L, -1, "OnExit");
        LUA_CHECK(lua_pcall(p_L, 0, 0, 0), "lua pcall error");
        lua_pop(p_L, 1);
    }
    p_L = nullptr;
}

void Script::mark_for_termination() {
    m_terminate = true;
}

void Script::update() {
    if (!p_L) { return; }
    if (m_terminate) {
        terminate();
    }

    const auto deltatime = Time::deltatime();

    if (m_funcs & (1 << (uint32_t)Callback::OnUpdate)) {
        lua_rawgeti(p_L, LUA_REGISTRYINDEX, m_env);
        lua_getfield(p_L, -1, "OnUpdate");
        lua_pushnumber(p_L, deltatime);
        LUA_CHECK(lua_pcall(p_L, 1, 0, 0), "lua pcall error");
        lua_pop(p_L, 1);
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
                } else {
#ifdef VANGO_DEBUG
                    std::cerr << "lua coroutine error - (internal) coroutine.yield nresults != 1\n";
                    exit(1);
#endif
                }
                break;
            case LUA_OK:
                async_start.thread = nullptr;
                async_start.resumable = false;
                break;
#ifdef VANGO_DEBUG
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
            lua_pushnumber(async_update.thread, (double)deltatime);
            switch (lua_resume(async_update.thread, 1)) {
            case LUA_YIELD: {
                if (const auto nresults = lua_gettop(async_update.thread); nresults == 1) {
                    async_update.resumable = true;
                    async_update.delay = static_cast<float>(lua_tonumber(async_update.thread, -1));
                    lua_pop(async_update.thread, nresults);
                } else {
#ifdef VANGO_DEBUG
                    std::cerr << "lua coroutine error - (internal) coroutine.yield nresults != 1\n";
                    exit(1);
#endif
                }
                break; }
            case LUA_OK:
                async_update.thread = nullptr;
                async_update.resumable = false;
                break;
#ifdef VANGO_DEBUG
            case LUA_ERRRUN:
                std::cerr << "lua coroutine error - " << lua_tostring(async_update.thread, -1) << "\n";
                exit(1);
#endif
            }
        }
    }
}



static void check_permutations(uint32_t funcs, Script::AsyncCallback* coroutines) {
    if (!(funcs & (1 << (uint32_t)Script::Callback::OnStart))  && !coroutines[(size_t)Script::Callback::OnStart].resumable &&
        !(funcs & (1 << (uint32_t)Script::Callback::OnUpdate)) && !coroutines[(size_t)Script::Callback::OnUpdate].resumable &&
        !(funcs & (1 << (uint32_t)Script::Callback::OnCreate)) &&
        !(funcs & (1 << (uint32_t)Script::Callback::OnExit))) {
        std::cerr << "lua parse error - script does not define any event handlers\n";
        exit(1);
    }
    if ((funcs & (1 << (uint32_t)Script::Callback::OnStart)) && coroutines[(size_t)Script::Callback::OnStart].resumable) {
        std::cerr << "lua parse error - 'OnStart' event handler is multiply defined\n";
        exit(1);
    }
    if ((funcs & (1 << (uint32_t)Script::Callback::OnUpdate)) && coroutines[(size_t)Script::Callback::OnUpdate].resumable) {
        std::cerr << "lua parse error - 'OnUpdate' event handler is multiply defined\n";
        exit(1);
    }
}

static int block_globals(lua_State* L) {
    const char* key = lua_tostring(L, 2);
    luaL_error(L, "attempted to access unsafe or nonexistent global: %s", key);
    return 0;
}

} }
