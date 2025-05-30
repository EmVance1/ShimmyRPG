#pragma once
#include <iostream>
#include <variant>
#include <unordered_map>


struct FlagAdd{ int dif; bool strict; };
struct FlagSub{ int dif; bool strict; };
struct FlagSet{ int val; bool strict; };
using FlagModifier = std::variant<FlagAdd, FlagSub, FlagSet>;


bool operator==(const FlagModifier& a, const FlagModifier& b);
std::ostream& operator<<(std::ostream& stream, const FlagModifier& p);


class FlagTable {
private:
    static std::unordered_map<std::string, uint32_t> cache;

public:
    inline static bool Once = false;

    static void change_flag(const std::string& key, const FlagModifier& mod) {
        if (const auto add = std::get_if<FlagAdd>(&mod)) {
            if (add->strict) {
                cache.at(key) += add->dif;
            } else {
                cache[key] += add->dif;
            }
        } else if (const auto sub = std::get_if<FlagSub>(&mod)) {
            if (sub->strict) {
                auto& flag = cache.at(key);
                flag = (sub->dif > (int)flag) ? 0 : (int)flag - sub->dif;
            } else {
                auto& flag = cache[key];
                flag = (sub->dif > (int)flag) ? 0 : (int)flag - sub->dif;
            }
        } else {
            const auto set = std::get<FlagSet>(mod);
            if (set.strict) {
                cache.at(key) = set.val;
            } else {
                cache[key] = set.val;
            }
        }
    }
    static void set_flag(const std::string& key, uint32_t val) {
        cache[key] = val;
    }
    static uint32_t get_flag(const std::string& key) {
        return cache[key];
    }
    static bool has_flag(const std::string& key) {
        return cache.find(key) != cache.end();
    }
    static void unset_flag(const std::string& key) {
        const auto it = cache.find(key);
        if (it != cache.end()) {
            cache.erase(it);
        }
    }
};

