#include "pch.h"
#include "flags.h"


bool operator==(const FlagMod& a, const FlagMod& b) {
    if (a.index() != b.index()) {
        return false;
    }
    if (const auto a_inc = std::get_if<FlagInc>(&a)) {
        const auto b_inc = std::get<FlagInc>(b);
        return a_inc->dif == b_inc.dif;
    } else {
        const auto a_set = std::get<FlagSet>(a);
        const auto b_set = std::get<FlagSet>(b);
        return a_set.val == b_set.val;
    }
}

std::ostream& operator<<(std::ostream& stream, const FlagMod& p) {
    if (const auto inc = std::get_if<FlagInc>(&p)) {
        if (inc->dif >= 0) {
            return stream << "+" << std::to_string(inc->dif);
        } else {
            return stream << std::to_string(inc->dif);
        }
    } else {
        const auto set = std::get<FlagSet>(p);
        return stream << std::to_string(set.val);
    }
}


std::unordered_map<std::string, int> FlagTable::cache;

