#include "pch.h"
#include "runtime.h"


namespace shmy { namespace lua {

Runtime::Runtime(Runtime&& other)
    : m_L(other.m_L), m_scripts(std::move(other.m_scripts)), m_api(std::move(other.m_api))
{
    other.m_L = nullptr;
}

Runtime::Runtime(const std::string& api) : m_api(api) {
    m_L = luaL_newstate();
    luaL_openlibs(m_L);
}

Runtime::~Runtime() {
    lua_close(m_L);
}


Script& Runtime::spawn_script(const std::filesystem::path& path) {
    return m_scripts.emplace_back(Script(m_L, path, m_api.c_str()));
}

void Runtime::update() {
    for (auto& s : m_scripts) {
        s.update();
    }
}

} }
