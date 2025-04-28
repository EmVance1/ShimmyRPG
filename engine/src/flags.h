#pragma once
#include <iostream>
#include <variant>
#include <unordered_map>


struct FlagAdd{ int dif; };
struct FlagSub{ int dif; };
struct FlagSet{ int val; };
using FlagModifier = std::variant<FlagAdd, FlagSub, FlagSet>;


bool operator==(const FlagModifier& a, const FlagModifier& b);
std::ostream& operator<<(std::ostream& stream, const FlagModifier& p);


class FlagTable {
private:
    static std::unordered_map<std::string, uint32_t> cache;

public:
    static void change_flag(const std::string& key, const FlagModifier& mod) {
        if (const auto add = std::get_if<FlagAdd>(&mod)) {
            cache[key] += add->dif;
        } else if (const auto sub = std::get_if<FlagSub>(&mod)) {
            if (sub->dif >= (int)cache[key]) {
                cache[key] = 0;
            } else {
                cache[key] -= sub->dif;
            }
        } else {
            cache[key] = std::get<FlagSet>(mod).val;
        }
    }
    static void set_flag(const std::string& key, uint32_t val) {
        cache[key] = val;
    }
    static uint32_t get_flag(const std::string& key) {
        return cache[key];
    }
};

