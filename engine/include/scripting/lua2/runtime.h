#pragma once
#include <luajit-2.1/lua.hpp>
#include <filesystem>
#include <unordered_map>
#include <vector>


namespace shmy { namespace lua2 {

class Runtime {
private:
    struct AsyncCallback {
        lua_State* thread = nullptr;
        bool resumable = false;
        float delay = 0;
    };

private:
    lua_State* m_L;
    std::string m_api;
    std::unordered_map<std::string, std::vector<int>> m_handlers;
    std::unordered_map<std::string, std::vector<AsyncCallback>> m_async_handlers;
    bool m_paused = false;

private:
    AsyncCallback init_coroutine(lua_State* L, const char* name);

public:
    Runtime(const std::string& api);
    Runtime(const Runtime&) = delete;
    Runtime(Runtime&& other);
    ~Runtime();

    void load_anon(const std::string& str);
    void load_file(const std::filesystem::path& path);

    void register_handler(const char* event, int func);
    void register_async_handler(const char* event, int func);

    const lua_State* get_state() const { return m_L; }
    lua_State* get_state() { return m_L; }

    void on_event(const std::string& event, int ref);
    void set_paused(bool paused);
};

} }
