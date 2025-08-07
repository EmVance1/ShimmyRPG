#pragma once
#include <iostream>
#include <variant>
#include <unordered_map>


struct FlagAdd{ int64_t dif; bool strict; };
struct FlagSub{ int64_t dif; bool strict; };
struct FlagSet{ int64_t val; bool strict; };
using FlagModifier = std::variant<FlagAdd, FlagSub, FlagSet>;


bool operator==(const FlagModifier& a, const FlagModifier& b);
std::ostream& operator<<(std::ostream& stream, const FlagModifier& p);


class FlagTable {
private:
    static std::unordered_map<std::string, uint64_t> cache;

public:
    inline static bool Never = false;

    static void clear() { cache.clear(); }

    static void change_flag(const std::string& key, const FlagModifier& mod);
    static void set_flag(const std::string& key, uint64_t val, bool strict);
    static uint64_t get_flag(const std::string& key, bool strict);
    static bool has_flag(const std::string& key);
    static void unset_flag(const std::string& key);
};

