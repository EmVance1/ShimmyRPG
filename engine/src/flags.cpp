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


std::unordered_map<std::string, uint64_t> FlagTable::cache;


void FlagTable::change_flag(const std::string& key, const FlagModifier& mod) {
    if (const auto add = std::get_if<FlagAdd>(&mod)) {
        if (add->strict) {
#ifdef DEBUG
        if (!has_flag(key)) {
            std::cerr << "runtime error: flag table does not contain key '" << key << "'\n";
            exit(1);
        }
#endif
            cache.at(key) += add->dif;
        } else {
            cache[key] += add->dif;
        }
    } else if (const auto sub = std::get_if<FlagSub>(&mod)) {
        if (sub->strict) {
#ifdef DEBUG
        if (!has_flag(key)) {
            std::cerr << "runtime error: flag table does not contain key '" << key << "'\n";
            exit(1);
        }
#endif
            auto& flag = cache.at(key);
            flag = (sub->dif > (int)flag) ? 0 : (int)flag - sub->dif;
        } else {
            auto& flag = cache[key];
            flag = (sub->dif > (int)flag) ? 0 : (int)flag - sub->dif;
        }
    } else {
        const auto set = std::get<FlagSet>(mod);
        if (set.strict) {
#ifdef DEBUG
        if (!has_flag(key)) {
            std::cerr << "runtime error: flag table does not contain key '" << key << "'\n";
            exit(1);
        }
#endif
            cache.at(key) = set.val;
        } else {
            cache[key] = set.val;
        }
    }
}

void FlagTable::set_flag(const std::string& key, uint64_t val, bool strict) {
    if (strict) {
#ifdef DEBUG
        if (!has_flag(key)) {
            std::cerr << "runtime error: flag table does not contain key '" << key << "'\n";
            exit(1);
        }
#endif
        cache.at(key) = val;
    } else {
        cache[key] = val;
    }
}

uint64_t FlagTable::get_flag(const std::string& key, bool strict) {
    if (strict) {
#ifdef DEBUG
        if (!has_flag(key)) {
            std::cerr << "runtime error: flag table does not contain key '" << key << "'\n";
            exit(1);
        }
#endif
        return cache.at(key);
    } else {
        return cache[key];
    }
}

bool FlagTable::has_flag(const std::string& key) {
    return cache.find(key) != cache.end();
}

void FlagTable::unset_flag(const std::string& key) {
    const auto it = cache.find(key);
    if (it != cache.end()) {
        cache.erase(it);
    }
}

