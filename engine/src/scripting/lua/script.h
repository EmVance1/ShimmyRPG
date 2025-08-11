#pragma once
#include <luajit-2.1/lua.hpp>
#include <filesystem>
#include <cstdint>


namespace shmy { namespace lua {

class Script {
public:
    enum class Callback {
        OnStart  = 0,
        OnUpdate = 1,
        OnCreate = 2,
        OnExit   = 3,
    };

    struct AsyncCallback {
        lua_State* thread = nullptr;
        bool resumable = false;
        float delay = 0;
    };

private:
    lua_State* p_L = nullptr;
    int m_env = 0;

    uint32_t m_funcs = 0;
    AsyncCallback m_coroutines[2];

    bool m_terminate = false;

private:
    void terminate();

public:
    Script(lua_State* L, const std::filesystem::path& filename, const char* api);
    ~Script();

    void start();
    void update();
    void mark_for_termination();
};

} }
