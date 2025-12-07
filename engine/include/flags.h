#pragma once
#include "scripting/expr.h"
#include <unordered_map>
#include <string>


class FlagTable {
public:
    struct Mod { enum { Add, Set } op; int64_t val; bool strict; };

private:
    static std::unordered_map<std::string, uint64_t> cache;
    static std::unordered_map<std::string, uint64_t> temps;
    static std::unordered_map<std::string, shmy::Expr> funcs;

public:
    inline static uint64_t Allow = 0;

    static void clear_all()   { cache.clear(); temps.clear(); funcs.clear(); }
    static void clear_temps() { temps.clear(); }

    static uint64_t* callback(const char* key, bool strict);

    static uint64_t get(const std::string& key, bool strict = true);
    static void set(const std::string& key, uint64_t val, bool strict = true);
    static bool has(const std::string& key);
    static void del(const std::string& key);
    static void mod(const std::string& key, const Mod& mod);
};

