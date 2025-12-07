#include "pch.h"
#include "flags.h"
#include "util/random.h"


std::unordered_map<std::string, uint64_t> FlagTable::cache;
std::unordered_map<std::string, uint64_t> FlagTable::temps;
std::unordered_map<std::string, shmy::Expr> FlagTable::funcs;

#define STR_EQ(s, l) (strncmp(s, l, sizeof(l)) == 0)


uint64_t* FlagTable::callback(const char* key, bool strict) {
    static uint64_t TEMP = 0;

    if (STR_EQ(key, "once")) {
        TEMP = FlagTable::Allow;
        return &TEMP;
    } else if (STR_EQ(key, "true")) {
        TEMP = 1;
        return &TEMP;
    } else if (STR_EQ(key, "false")) {
        TEMP = 0;
        return &TEMP;
    } else if (STR_EQ(key, "inf")) {
        TEMP = UINT64_MAX;
        return &TEMP;
    } else if (STR_EQ(key, "default")) {
        TEMP = 1;
        return &TEMP;
    } else if (strncmp(key, "rng", 3) == 0) {
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

void FlagTable::mod(const std::string& key, const Mod& mod) {
    auto ptr = FlagTable::callback(key.c_str(), mod.strict);
    switch (mod.op) {
    case Mod::Add:
        if (mod.val < 0 && (uint64_t)std::abs(mod.val) > *ptr) {
            *ptr = 0;
        } else {
            *ptr += (uint64_t)mod.val;
        }
        break;
    case Mod::Set:
        *ptr = (uint64_t)mod.val;
        break;
    }
}

void FlagTable::set(const std::string& key, uint64_t val, bool strict) {
    *FlagTable::callback(key.c_str(), strict) = val;
}

uint64_t FlagTable::get(const std::string& key, bool strict) {
    return *FlagTable::callback(key.c_str(), strict);
}

bool FlagTable::has(const std::string& key) {
    return cache.contains(key) || temps.contains(key) || funcs.contains(key);
}

void FlagTable::del(const std::string& key) {
    cache.erase(key);
    temps.erase(key);
    funcs.erase(key);
}

