#include <iostream>
#include <variant>
#include <unordered_map>


struct FlagInc{ int dif; };
struct FlagSet{ int val; };
using FlagMod = std::variant<FlagInc, FlagSet>;


bool operator==(const FlagMod& a, const FlagMod& b);
std::ostream& operator<<(std::ostream& stream, const FlagMod& p);


class FlagTable {
private:
    static std::unordered_map<std::string, int> cache;

private:
    static void change_flag(const std::string& key, const FlagMod& mod) {
        if (const auto inc = std::get_if<FlagInc>(&mod)) {
            cache[key] += inc->dif;
        } else {
            const auto set = std::get<FlagSet>(mod);
            cache[key] = set.val;
        }
    }
    static void set_flag(const std::string& key, const int& val) {
        cache[key] = val;
    }

    static int get_flag(const std::string& key) {
        return cache[key];
    }
};

