#include "luajit-2.1/lua.h"
#include "pch.h"
#include "scripting/lua2/runtime.h"
#include "core/fs.h"


namespace shmy { namespace lua2 {

#ifdef VANGO_DEBUG
#define LUA_CHECK(f, pre) if (f != LUA_OK) { \
        std::cerr << pre << " - " << lua_tostring(m_L, -1) << "\n"; \
        exit(1); \
    }
#else
#define LUA_CHECK(f, pre) f
#endif

static void init_env(lua_State* m_L, const char* api, const char* str);
int l_register_handler(lua_State* L);
int l_register_async_handler(lua_State* L);


Runtime::Runtime(const std::string& api) : m_api(api) {
    m_L = luaL_newstate();
    luaL_openlibs(m_L);

    lua_pushlightuserdata(m_L, this);
    lua_setfield(m_L, LUA_REGISTRYINDEX, "_runtime");

    lua_pushcfunction(m_L, &l_register_handler);
    lua_setglobal(m_L, "RegisterHandler");

    lua_pushcfunction(m_L, &l_register_async_handler);
    lua_setglobal(m_L, "RegisterAsyncHandler");
}

Runtime::Runtime(Runtime&& other)
    : m_L(other.m_L),
    m_api(std::move(other.m_api)),
    m_handlers(std::move(other.m_handlers)),
    m_async_handlers(std::move(other.m_async_handlers))
{
    other.m_L = nullptr;
}

Runtime::~Runtime() {
    lua_close(m_L);
}


void Runtime::load_anon(const std::string& str) {
    init_env(m_L, m_api.c_str(), str.c_str());
}

void Runtime::load_file(const std::filesystem::path& path) {
    const auto file = core::read_to_string(path).unwrap();
    init_env(m_L, m_api.c_str(), file.c_str());
}

void Runtime::register_handler(const char* event, int func) {
    m_handlers[event].push_back(func);
}

void Runtime::register_async_handler(const char* event, int func) {
    auto cb = Runtime::AsyncCallback{};
    cb.thread = lua_newthread(m_L); // env, co
    cb.resumable = true;
    cb.delay = 0.f;
    lua_rawgeti(m_L, LUA_REGISTRYINDEX, func); // env, co, f   |
    lua_xmove(m_L, cb.thread, 1);              // env, co      | f
    lua_pop(m_L, 1);                           // env          | f
    m_async_handlers[event].push_back(cb);
}


void Runtime::on_event(const std::string& event, int ref) {
    if (!m_L) { return; }

    for (int f : m_handlers[event]) {
        lua_rawgeti(m_L, LUA_REGISTRYINDEX, f);    // f
        lua_rawgeti(m_L, LUA_REGISTRYINDEX, ref);  // f, args
        LUA_CHECK(lua_pcall(m_L, 0, 0, 0), "lua pcall error");
    }

    for (auto& f : m_async_handlers[event]) {
        if (f.resumable) {
            lua_rawgeti(m_L, LUA_REGISTRYINDEX, ref);
            lua_xmove(m_L, f.thread, 1);
            switch (lua_resume(f.thread, 1)) {
            case LUA_YIELD:
                if (const auto nresults = lua_gettop(f.thread); nresults == 1) {
                    f.resumable = true;
                    f.delay = (float)lua_tonumber(f.thread, -1);
                    lua_pop(f.thread, nresults);
                }
#ifdef VANGO_DEBUG
                else {
                    std::cerr << "lua coroutine error - (internal) coroutine.yield nresults != 1\n";
                    exit(1);
                }
#endif
                break;
            case LUA_OK:
                f.thread = nullptr;
                f.resumable = false;
                break;

#ifdef VANGO_DEBUG
            case LUA_ERRRUN:
                std::cerr << "lua coroutine error - " << lua_tostring(f.thread, -1) << "\n";
                exit(1);
#endif
            }
        }
    }
}

void Runtime::set_paused(bool paused) {
    m_paused = paused;
}


static int block_globals(lua_State* L) {
    const char* key = lua_tostring(L, 2);
    luaL_error(L, "attempted to access unsafe or nonexistent global: %s", key);
    return 0;
}

static void init_env(lua_State* m_L, const char* api, const char* str) {
    LUA_CHECK(luaL_loadstring(m_L, str), "lua parse error"); // chunk

    lua_newtable(m_L); // chunk, env
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

    lua_newtable(m_L);                     // chunk, env, meta
    lua_pushcfunction(m_L, block_globals);
    lua_setfield(m_L, -2, "__index");      // chunk, env, meta
    lua_setmetatable(m_L, -2);             // chunk, env

    lua_setfenv(m_L, -3);                  // chunk

    LUA_CHECK(lua_pcall(m_L, 0, 0, 0), "lua pcall error");
}

Runtime::AsyncCallback Runtime::init_coroutine(lua_State* L, const char* name) {
    auto cb = Runtime::AsyncCallback{};
    cb.thread = lua_newthread(L); // env, co
    cb.resumable = true;
    cb.delay = 0.f;
    lua_getfield(L, -2, name);    // env, co, f   |
    lua_xmove(L, cb.thread, 1);   // env, co      | f
    lua_pop(L, 1);                // env          | f
    return cb;
}


} }

