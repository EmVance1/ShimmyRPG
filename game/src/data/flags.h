#pragma once
#include <unordered_map>
#include <unordered_set>
#include <filesystem>
#include <string>


namespace shmy::data {

class Flags {
public:
    struct Mod { enum { Add, Set } op; int64_t val; };

private:
    static std::vector<uint64_t> cache;
    static std::unordered_map<std::string, uint32_t> cache_lkp;
    static std::unordered_set<std::string> once_set;
    // static std::unordered_map<std::string, shmy::Expr> funcs;

public:
    static void init(const std::filesystem::path& path);
    static void reset() { cache.clear(); cache_lkp.clear(); }

    static uint32_t key_hook(const char* key);
    static uint64_t* value_hook(uint32_t key);
    static uint64_t* kv_hook(const char* key);

    static uint32_t create(const std::string& key);
    static uint64_t get(uint32_t key);
    static uint64_t get(const std::string& key);
    static void set(uint32_t key, uint64_t val);
    static void set(const std::string& key, uint64_t val);
    static void mod(uint32_t key, const Mod& mod);
    static void mod(const std::string& key, const Mod& mod);
    static bool has(const std::string& key);

    static bool check_once(const std::string& key);
    static void set_once(const std::string& key);
    static uint64_t& Allow();
};

}
