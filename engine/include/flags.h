#pragma once
#include <unordered_map>
#include <variant>
#include <string>


struct FlagAdd{ int64_t val; bool strict; };
struct FlagSet{ int64_t val; bool strict; };
using FlagModifier = std::variant<FlagAdd, FlagSet>;


class FlagTable {
private:
    static std::unordered_map<std::string, uint64_t> cache;

public:
    inline static uint64_t Never = 0;

    static void clear() { cache.clear(); }

    static uint64_t* callback(const char* key, bool strict);

    static void change_flag(const std::string& key, const FlagModifier& mod);
    static void set_flag(const std::string& key, uint64_t val, bool strict);
    static uint64_t get_flag(const std::string& key, bool strict);
    static bool has_flag(const std::string& key);
    static void unset_flag(const std::string& key);
};

