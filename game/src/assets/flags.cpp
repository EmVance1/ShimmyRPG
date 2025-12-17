#include "pch.h"
#include "flags.h"
#include "core/split.h"
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
#ifdef SHMY_DEBUG
            if (strict) {
                std::cerr << "runtime error - invalid flag table key '" << key << "'\n";
                exit(1);
            }
#endif
            temps[key] = 0;
        }
        return &temps.at(key);
    } else if (!cache.contains(key)) {
#ifdef SHMY_DEBUG
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


static void load_dir(const std::fs::path& dir, const std::string& path);

void FlagTable::init(const std::fs::path& dir) {
    load_dir(dir, "");
}


static std::pair<std::string, std::string> split_kv(const std::string& str, const std::fs::path& filename, size_t line, bool& err) {
    for (size_t i = 0; i < str.size(); i++) {
        if (str[i] == '=') {
            const auto key = str.substr(0, i);
            const auto val = str.substr(i+1);
            return { shmy::core::trim(key), shmy::core::trim(val) };
        }
    }

    std::cerr << "load module error - flag initializer must contain '<key> = <val>', " <<
        filename.generic_string() << ":" << line << "\n";
    err = true;
    return {};
}

static void load_file(const std::fs::path& filename, const std::string& path, const std::string& file) {
    auto f   = std::ifstream(filename);
    auto str = std::string("");
    size_t line = 1;
    bool err = false;

    while (std::getline(f, str)) {
        if (shmy::core::trim(str).empty() || (str[0] == '#')) {
            continue;
        }
        auto [key, val] = split_kv(str, filename, line, err);
        if (key == "true" || key == "false" || key == "inf" || key == "default") {
            std::cerr << "load module error - '" << key << "' is a reserved value and cannot be overridden, " <<
                filename.generic_string() << ":" << line << "\n";
            err = true;
            continue;
        }
        if (key.starts_with("rng")) {
            std::cerr << "load module error - 'rng' is a reserved prefix and cannot be overridden, " <<
                filename.generic_string() << ":" << line << "\n";
            err = true;
            continue;
        }
        if (key.starts_with("once")) {
            std::cerr << "load module error - 'once' is a reserved prefix and cannot be overridden, " <<
                filename.generic_string() << ":" << line << "\n";
            err = true;
            continue;
        }

        const auto p = path.empty() ? "" : path + ".";
        if (file == "Global") {
            key = p + key;
        } else {
            key = p + file + "." + key;
        }
        // std::cout << "loaded key: " << key << "\n";
        if (val == "true") {
            FlagTable::set(key, 1, false);
        } else if (val == "false") {
            FlagTable::set(key, 0, false);
        } else if (val == "inf") {
            FlagTable::set(key, UINT64_MAX, false);
        } else {
            FlagTable::set(key, (uint64_t)std::atoll(val.c_str()), false);
        }
        line++;
    }

    if (err) { exit(1); }
}

static void load_dir(const std::fs::path& dir, const std::string& path) {
    for (const auto& f : std::fs::directory_iterator(dir)) {
        const auto stem = f.path().stem().string();
        if (f.is_directory()) {
            load_dir(f.path(), path.empty() ? stem : path + "." + stem);
        } else {
            load_file(f.path(), path, stem);
        }
    }
}

