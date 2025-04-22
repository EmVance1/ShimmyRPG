#include "pch.h"
#include "flags.h"


bool operator==(const FlagModifier& a, const FlagModifier& b) {
    if (a.index() != b.index()) {
        return false;
    }
    if (const auto a_add = std::get_if<FlagAdd>(&a)) {
        const auto b_add = std::get<FlagAdd>(b);
        return a_add->dif == b_add.dif;
    } else if (const auto a_sub = std::get_if<FlagSub>(&a)) {
        const auto b_sub = std::get<FlagSub>(b);
        return a_sub->dif == b_sub.dif;
    } else {
        const auto a_set = std::get<FlagSet>(a);
        const auto b_set = std::get<FlagSet>(b);
        return a_set.val == b_set.val;
    }
}

std::ostream& operator<<(std::ostream& stream, const FlagModifier& p) {
    if (const auto add = std::get_if<FlagAdd>(&p)) {
        return stream << "Add(" << std::to_string(add->dif) << ")";
    } else if (const auto sub = std::get_if<FlagSub>(&p)) {
        return stream << "Sub(" << std::to_string(sub->dif) << ")";
    } else {
        const auto set = std::get<FlagSet>(p);
        return stream << "Set(" << std::to_string(set.val) << ")";
    }
}


std::unordered_map<std::string, uint32_t> FlagTable::cache;

