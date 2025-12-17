#pragma once
#include <luajit-2.1/lua.hpp>
#include <filesystem>
#include <unordered_map>
#include <vector>


namespace shmy { namespace lua {

struct EventArgs {
    bool manage;
    int ref;
};

class Runtime {
public:
    struct Callback {
        int func;
        int state;
    };
    struct AsyncCallback {
        lua_State* thread = nullptr;
        bool in_prog = false;
        float delay = 0;
        int state;
    };

private:
    lua_State* m_L;
    int m_sandboxref = 0;
    std::unordered_map<std::string, std::vector<Callback>> m_handlers;
    std::unordered_map<std::string, std::vector<AsyncCallback>> m_async_handlers;
    std::vector<int> m_states;
    bool m_paused = false;

public:
    Runtime();
    Runtime(const Runtime&) = delete;
    Runtime(Runtime&& other) noexcept;
    ~Runtime();

    void init_env(void(*init_api)(lua_State*), const std::string& api);

    void load_anon(const std::string& str);
    void load_file(const std::filesystem::path& path);

    void register_handler(const char* event, Callback cb);
    void register_handler(const char* event, AsyncCallback cb);

    const lua_State* get_state() const { return m_L; }
    lua_State* get_state() { return m_L; }

    void on_event(const std::string& event, EventArgs args);
    void set_paused(bool paused);
    void update();
};

} }
