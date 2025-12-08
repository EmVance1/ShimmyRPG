#include "luajit-2.1/lua.h"
#include "pch.h"
#include "scripting/lua/runtime.h"
#include "core/fs.h"
#include "util/deltatime.h"


namespace shmy { namespace lua {

#ifdef VANGO_DEBUG
#define LUA_CHECK(f, pre) if (f != LUA_OK) { \
        std::cerr << pre << " - " << lua_tostring(m_L, -1) << "\n"; \
        exit(1); \
    }
#else
#define LUA_CHECK(f, pre) f
#endif

static int s_init_env(lua_State* m_L, const char* api);
int l_register_handler(lua_State* L);
int l_register_async_handler(lua_State* L);
int l_dispatch_event(lua_State* L);


Runtime::Runtime() {
    m_L = luaL_newstate();
    luaL_openlibs(m_L);

    lua_pushlightuserdata(m_L, this);
    lua_setfield(m_L, LUA_REGISTRYINDEX, "_runtime");

    luaL_dostring(m_L,
        "function _AsyncWrapper(worker)\n"
        "    while true do\n"
        "       state, args = coroutine.yield(-1)\n"
        "       worker(state, args)\n"
        "    end\n"
        "end\n"
    );
}

Runtime::Runtime(Runtime&& other)
    : m_L(other.m_L),
    m_handlers(std::move(other.m_handlers)),
    m_async_handlers(std::move(other.m_async_handlers))
{
    other.m_L = nullptr;
}

Runtime::~Runtime() {
    lua_close(m_L);
}


void Runtime::init_env(void(*init_api)(lua_State*), const std::string& api) {
    init_api(m_L);
    m_sandboxref = s_init_env(m_L, api.c_str());
}

void Runtime::load_anon(const std::string& str) {
    LUA_CHECK(luaL_loadstring(m_L, str.c_str()), "lua parse error");
    lua_rawgeti(m_L, LUA_REGISTRYINDEX, m_sandboxref);
    lua_setfenv(m_L, -2);

    LUA_CHECK(lua_pcall(m_L, 0, 0, 0), "lua pcall error");
}

void Runtime::load_file(const std::filesystem::path& path) {
    const auto file = core::read_to_string(path).unwrap();
    LUA_CHECK(luaL_loadstring(m_L, file.c_str()), "lua parse error");
    lua_rawgeti(m_L, LUA_REGISTRYINDEX, m_sandboxref);
    lua_setfenv(m_L, -2);

    lua_newtable(m_L);
    const int locstate = luaL_ref(m_L, LUA_REGISTRYINDEX);
    lua_pushinteger(m_L, locstate);
    lua_setfield(m_L, LUA_REGISTRYINDEX, "_locstate");

    LUA_CHECK(lua_pcall(m_L, 0, 0, 0), "lua pcall error");
}

void Runtime::register_handler(const char* event, Callback cb) {
    m_handlers[event].push_back(cb);
}

void Runtime::register_async_handler(const char* event, AsyncCallback cb) {
    m_async_handlers[event].push_back(cb);
}


void Runtime::on_event(const std::string& event, EventArgs args) {
    if (m_paused) return;

    for (auto& cb : m_handlers[event]) {
        lua_rawgeti(m_L, LUA_REGISTRYINDEX, cb.func);
        lua_rawgeti(m_L, LUA_REGISTRYINDEX, cb.state);
        if (args.ref == -1) {
            lua_pushnil(m_L);
        } else {
            lua_rawgeti(m_L, LUA_REGISTRYINDEX, args.ref);
        }
        LUA_CHECK(lua_pcall(m_L, 2, 0, 0), "lua pcall error");
    }

    for (auto& cb : m_async_handlers[event]) {
        if (!cb.thread || cb.in_prog) continue;

        lua_rawgeti(m_L, LUA_REGISTRYINDEX, cb.state);     // s    |
        if (args.ref == -1) {
            lua_xmove(m_L, cb.thread, 1);                  //      | s
            lua_pushnil(cb.thread);                        //      | s, nil
        } else {
            lua_rawgeti(m_L, LUA_REGISTRYINDEX, args.ref); // s, a |
            lua_xmove(m_L, cb.thread, 2);                  //      | s, a
        }
        switch (lua_resume(cb.thread, 2)) {
        case LUA_YIELD:
            if (const auto nresults = lua_gettop(cb.thread); nresults == 1) {
                cb.delay = (float)lua_tonumber(cb.thread, -1);
                if (cb.delay >= 0) {
                    cb.in_prog = true;
                } else {
                    cb.in_prog = false;
                    cb.delay = 0;
                }
                lua_pop(cb.thread, nresults);
                break;
            }
#ifdef VANGO_DEBUG
            std::cerr << "lua coroutine error - (internal) coroutine.yield nresults != 1\n";
            exit(1);
        case LUA_OK:
            std::cerr << "lua coroutine error - (internal) coroutine exit is not intended behaviour\n";
            exit(1);
        case LUA_ERRRUN:
            std::cerr << "lua coroutine error - " << lua_tostring(cb.thread, -1) << "\n";
            exit(1);
#endif
        }
    }

    if (args.manage) {
        luaL_unref(m_L, LUA_REGISTRYINDEX, args.ref);
    }
}

void Runtime::set_paused(bool paused) {
    m_paused = paused;
}

void Runtime::update() {
    if (m_paused) return;

    const auto deltatime = Time::deltatime();

    for (auto& cb : m_handlers["OnUpdate"]) {
        lua_rawgeti(m_L, LUA_REGISTRYINDEX, cb.func);
        lua_rawgeti(m_L, LUA_REGISTRYINDEX, cb.state);
        lua_pushnumber(m_L, (lua_Number)deltatime);
        LUA_CHECK(lua_pcall(m_L, 2, 0, 0), "lua pcall error");
    }

    for (auto& cb : m_async_handlers["OnUpdate"]) {
        if (!cb.thread || cb.in_prog) continue;
                                                           //   |
        lua_rawgeti(m_L, LUA_REGISTRYINDEX, cb.state);     // s |
        lua_xmove(m_L, cb.thread, 1);                      //   | s
        lua_pushnumber(cb.thread, (lua_Number)deltatime);  //   | s, dt
        switch (lua_resume(cb.thread, 2)) {
        case LUA_YIELD:
            if (const auto nresults = lua_gettop(cb.thread); nresults == 1) {
                cb.delay = (float)lua_tonumber(cb.thread, -1);
                if (cb.delay >= 0) {
                    cb.in_prog = true;
                } else {
                    cb.in_prog = false;
                    cb.delay = 0;
                }
                lua_pop(cb.thread, nresults);
                break;
            }
#ifdef VANGO_DEBUG
            std::cerr << "lua coroutine error - (internal) coroutine.yield nresults != 1\n";
            exit(1);
        case LUA_OK:
            std::cerr << "lua coroutine error - (internal) coroutine exit is not intended behaviour\n";
            exit(1);
        case LUA_ERRRUN:
            std::cerr << "lua coroutine error - " << lua_tostring(cb.thread, -1) << "\n";
            exit(1);
#endif
        }
    }

    for (auto& [ev, handlers] : m_async_handlers) {
        for (auto& cb : handlers) {
            if (!cb.thread || !cb.in_prog) continue;
            cb.delay -= deltatime;
            if (cb.delay > 0) continue;

            switch (lua_resume(cb.thread, 0)) {
            case LUA_YIELD:
                if (const auto nresults = lua_gettop(cb.thread); nresults == 1) {
                    cb.delay = (float)lua_tonumber(cb.thread, -1);
                    if (cb.delay >= 0) {
                        cb.in_prog = true;
                    } else {
                        cb.in_prog = false;
                        cb.delay = 0;
                    }
                    lua_pop(cb.thread, nresults);
                    break;
                }
#ifdef VANGO_DEBUG
                std::cerr << "lua coroutine error - (internal) coroutine.yield nresults != 1\n";
                exit(1);
            case LUA_OK:
                std::cerr << "lua coroutine error - (internal) coroutine exit is not intended behaviour\n";
                exit(1);
            case LUA_ERRRUN:
                std::cerr << "lua coroutine error - " << lua_tostring(cb.thread, -1) << "\n";
                exit(1);
#endif
            }
        }
    }
}


static int block_globals(lua_State* L) {
    const char* key = lua_tostring(L, 2);
    luaL_error(L, "attempted to access unsafe or nonexistent global: %s", key);
    return 1;
}

static int s_init_env(lua_State* m_L, const char* api) {
    lua_newtable(m_L); // env

    lua_pushcfunction(m_L, &l_register_handler);
    lua_setfield(m_L, -2, "RegisterHandler");
    lua_pushcfunction(m_L, &l_register_async_handler);
    lua_setfield(m_L, -2, "RegisterAsyncHandler");
    lua_pushcfunction(m_L, &l_dispatch_event);
    lua_setfield(m_L, -2, "DispatchEvent");

    lua_getglobal(m_L, api);
    lua_setfield(m_L, -2, api);
    lua_getglobal(m_L, "math");
    lua_setfield(m_L, -2, "math");
    lua_getglobal(m_L, "string");
    lua_setfield(m_L, -2, "string");
    lua_getglobal(m_L, "table");
    lua_setfield(m_L, -2, "table");
    lua_getglobal(m_L, "coroutine");
    lua_setfield(m_L, -2, "coroutine");

    lua_getglobal(m_L, "print");
    lua_setfield(m_L, -2, "print");
    lua_getglobal(m_L, "pcall");
    lua_setfield(m_L, -2, "pcall");
    lua_getglobal(m_L, "pairs");
    lua_setfield(m_L, -2, "pairs");
    lua_getglobal(m_L, "ipairs");
    lua_setfield(m_L, -2, "ipairs");
    lua_getglobal(m_L, "tonumber");
    lua_setfield(m_L, -2, "tonumber");
    lua_getglobal(m_L, "tostring");
    lua_setfield(m_L, -2, "tostring");
    lua_getglobal(m_L, "next");
    lua_setfield(m_L, -2, "next");
    lua_getglobal(m_L, "select");
    lua_setfield(m_L, -2, "select");
    lua_getglobal(m_L, "unpack");
    lua_setfield(m_L, -2, "unpack");

    lua_newtable(m_L);                     // env, meta
    lua_pushcfunction(m_L, block_globals); // env, meta, block
    lua_setfield(m_L, -2, "__index");      // env, meta
    lua_setmetatable(m_L, -2);             // env

    return luaL_ref(m_L, LUA_REGISTRYINDEX);
}


} }

