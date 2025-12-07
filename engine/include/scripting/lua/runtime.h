#pragma once
#include <luajit-2.1/lua.hpp>
#include <filesystem>
#include <vector>
#include "script.h"


namespace shmy { namespace lua {

class Runtime {
private:
    lua_State* m_L;
    std::vector<Script> m_scripts;
    std::string m_api;
    bool m_paused = false;

public:
    Runtime(const Runtime&) = delete;
    Runtime(Runtime&& other);
    Runtime(const std::string& api);
    ~Runtime();

    Script& spawn_script(const std::filesystem::path& path);
    void spawn_small(const std::string& str);

    void update();
    void set_paused(bool paused);

    const lua_State* get_state() const { return m_L; }
    lua_State* get_state() { return m_L; }
};

} }
