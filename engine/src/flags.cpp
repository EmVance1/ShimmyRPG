#include "pch.h"
#include "flags.h"
#include "util/random.h"


std::unordered_map<std::string, uint64_t> FlagTable::cache;
std::unordered_map<std::string, uint64_t> FlagTable::temps;
std::unordered_map<std::string, shmy::Expr> FlagTable::funcs;


uint64_t* FlagTable::callback(const char* key, bool strict) {
    static uint64_t TEMP = 0;
    const auto len = strlen(key);

    if (len == 0) {
        std::cerr << "runtime error - invalid flag table key ''\n";
        exit(1);
    } else if (strcmp(key, "once") == 0) {
        TEMP = FlagTable::Allow;
        return &TEMP;
    } else if (strcmp(key, "true") == 0) {
        TEMP = 1;
        return &TEMP;
    } else if (strcmp(key, "false") == 0) {
        TEMP = 0;
        return &TEMP;
    } else if (strcmp(key, "inf") == 0) {
        TEMP = UINT64_MAX;
        return &TEMP;
    } else if (strcmp(key, "default") == 0) {
        TEMP = 1;
        return &TEMP;
    } else if (strcmp(key, "rng") == 0) {
        TEMP = (uint64_t)Random::integer(0, atoll(key+3)-1);
        return &TEMP;
    } else if (key[0] == '_') {
        if (!temps.contains(key)) {
#ifdef VANGO_DEBUG
            if (strict) {
                std::cerr << "runtime error - invalid flag table key '" << key << "'\n";
                exit(1);
            }
#endif
            temps[key] = 0;
        }
        return &temps.at(key);
    } else if (!cache.contains(key)) {
#ifdef VANGO_DEBUG
        if (strict) {
            std::cerr << "runtime error - invalid flag table key '" << key << "'\n";
            exit(1);
        }
#endif
        cache[key] = 0;
    }
    return &cache.at(key);
}

void FlagTable::change_flag(const std::string& key, const FlagModifier& mod) {
    if (const auto add = std::get_if<FlagAdd>(&mod)) {
        auto ptr = FlagTable::callback(key.c_str(), add->strict);
        if (add->val < 0 && (uint64_t)std::abs(add->val) > *ptr) {
            *ptr = 0;
        } else {
            *ptr += (uint64_t)add->val;
        }
    } else {
        const auto set = std::get<FlagSet>(mod);
        auto ptr = FlagTable::callback(key.c_str(), set.strict);
        *ptr = (uint64_t)set.val;
    }
}

void FlagTable::set_flag(const std::string& key, uint64_t val, bool strict) {
    *FlagTable::callback(key.c_str(), strict) = val;
}

uint64_t FlagTable::get_flag(const std::string& key, bool strict) {
    return *FlagTable::callback(key.c_str(), strict);
}

bool FlagTable::has_flag(const std::string& key) {
    return cache.contains(key);
}

void FlagTable::unset_flag(const std::string& key) {
    cache.erase(key);
}

