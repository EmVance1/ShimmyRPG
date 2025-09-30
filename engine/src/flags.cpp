#include "pch.h"
#include "flags.h"
#include <random>


static std::mt19937 RNG{ std::random_device()() };


std::unordered_map<std::string, uint64_t> FlagTable::cache;


uint64_t* FlagTable::callback(const char* key, bool strict) {
    static uint64_t TEMP = 0;

    if (strncmp(key, "once", 4) == 0) {
        return &FlagTable::Never;
    } else if (strncmp(key, "default", 7) == 0) {
        TEMP = 1;
        return &TEMP;
    } else if (strncmp(key, "rng", 3) == 0) {
        auto mod = (uint64_t)atoll(key+3);
        auto dist = std::uniform_int_distribution<uint64_t>(0, mod-1);
        TEMP = dist(RNG);
        return &TEMP;
    } else {
        if (!strict && !cache.contains(key)) {
            cache[key] = 0;
        }
        return &cache.at(key);
    }
}

void FlagTable::change_flag(const std::string& key, const FlagModifier& mod) {
    if (const auto add = std::get_if<FlagAdd>(&mod)) {
        auto val = 0ULL;
        if (add->strict) {
#ifdef VANGO_DEBUG
            if (!has_flag(key)) {
                std::cerr << "runtime error: flag table does not contain key '" << key << "'\n";
                exit(1);
            }
#endif
            val = cache.at(key);
        } else {
            val = cache[key];
        }
        if (add->val < 0 && (uint64_t)std::abs(add->val) > val) {
            cache[key] = 0;
        } else {
            cache[key] = val + (uint64_t)add->val;
        }
    } else {
        const auto set = std::get<FlagSet>(mod);
        if (set.strict) {
#ifdef VANGO_DEBUG
            if (!has_flag(key)) {
                std::cerr << "runtime error: flag table does not contain key '" << key << "'\n";
                exit(1);
            }
#endif
            cache.at(key) = (uint64_t)set.val;
        } else {
            cache[key] = (uint64_t)set.val;
        }
    }
}

void FlagTable::set_flag(const std::string& key, uint64_t val, bool strict) {
    if (strict) {
#ifdef VANGO_DEBUG
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
#ifdef VANGO_DEBUG
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

