#include "pch.h"
#include "flags.h"
#include "core/split.h"
#include "util/random.h"


namespace shmy::data {

std::vector<uint64_t> Flags::cache;
std::unordered_map<std::string, uint32_t> Flags::cache_lkp;
uint32_t Flags::temps_ptr;
std::unordered_map<std::string, uint32_t> Flags::temps_lkp;
std::unordered_set<std::string> Flags::once_set;

enum class Reserved : uint32_t {
    False,
    True,
    Once,
    Inf,
    Rng = (1u << 31),
};

#define STR_EQ(s, l) (strncmp(s, l, sizeof(l)) == 0)


uint64_t* Flags::value_hook(uint32_t key) {
    if ((uint32_t)Reserved::Rng & key) {
        static uint64_t TEMP = 0;
        TEMP = (uint64_t)Random::integer(0, (uint32_t)Reserved::Rng ^ key);
        return &TEMP;
    } else {
        return &cache[key];
    }
}
uint32_t Flags::key_hook(const char* key) {
    if (strncmp(key, "rng", 3) == 0) {
        const uint32_t mod = (uint32_t)Random::integer(0, atoll(key+3)-1);
        return (uint32_t)Reserved::Rng | mod;
    } else if (key[0] == '_') {
        if (!temps_lkp.contains(key)) {
            temps_lkp[key] = (uint32_t)temps_ptr;
            cache[temps_ptr++] = 0;
        }
        return temps_lkp.at(key);
    } else {
        if (!cache_lkp.contains(key)) {
            std::cerr << "runtime error - invalid flag table key '" << key << "'\n";
            exit(1);
        }
        return cache_lkp.at(key);
    }
}

uint64_t* Flags::kv_hook(const char* key) {
    return value_hook(key_hook(key));
}


void Flags::mod(const std::string& key, const Mod& mod) {
    Flags::mod(cache_lkp.at(key), mod);
}
void Flags::mod(const uint32_t key, const Mod& mod) {
    auto ptr = Flags::value_hook(key);
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

void Flags::set(uint32_t key, uint64_t val) {
    *Flags::value_hook(key) = val;
}
void Flags::set(const std::string& key, uint64_t val) {
    Flags::set(cache_lkp.at(key), val);
}

uint64_t Flags::get(uint32_t key) {
    return *Flags::value_hook(key);
}
uint64_t Flags::get(const std::string& key) {
    return Flags::get(cache_lkp.at(key));
}

bool Flags::has(const std::string& key) {
    return cache_lkp.contains(key);
}


bool Flags::check_once(const std::string& key) {
    return once_set.contains(key);
}
void Flags::set_once(const std::string& key) {
    once_set.insert(key);
}
uint64_t& Flags::Allow() {
    return cache[2];
}


static void load_dir(const std::fs::path& dir, const std::string& path, std::vector<uint64_t>& cache, std::unordered_map<std::string, uint32_t>& cache_lkp);

void Flags::init(const std::fs::path& dir) {
    cache.push_back(0);          // false
    cache.push_back(1);          // true
    cache.push_back(0);          // once
    cache.push_back(UINT64_MAX); // inf
    cache_lkp["false"] = 0;
    cache_lkp["true"]  = 1;
    cache_lkp["once"]  = 2;
    cache_lkp["inf"]   = 3;
    load_dir(dir, "", cache, cache_lkp);
    temps_ptr = (uint32_t)cache.size();
    cache.resize(cache.size() + MAX_TEMPS);
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

static void load_file(const std::fs::path& filename, const std::string& path, const std::string& file,
    std::vector<uint64_t>& cache, std::unordered_map<std::string, uint32_t>& cache_lkp)
{

    auto f   = std::ifstream(filename);
    auto str = std::string("");
    size_t line = 1;
    bool err = false;


    while (std::getline(f, str)) {
        if (shmy::core::trim(str).empty() || (str[0] == '#')) {
            continue;
        }
        auto [key, val] = split_kv(str, filename, line, err);
        if (key == "true" || key == "false" || key == "inf") {
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

        const size_t idx = cache.size();
        if (val == "true") {
            cache.push_back(1);
        } else if (val == "false") {
            cache.push_back(0);
        } else if (val == "inf") {
            cache.push_back(UINT64_MAX);
        } else {
            cache.push_back((uint64_t)std::atoll(val.c_str()));
        }
        cache_lkp[key] = (uint32_t)idx;
        line++;
    }

    if (err) { exit(1); }
}

static void load_dir(const std::fs::path& dir, const std::string& path,
    std::vector<uint64_t>& cache, std::unordered_map<std::string, uint32_t>& cache_lkp)
{
    for (const auto& f : std::fs::directory_iterator(dir)) {
        const auto stem = f.path().stem().string();
        if (f.is_directory()) {
            load_dir(f.path(), path.empty() ? stem : path + "." + stem, cache, cache_lkp);
        } else {
            load_file(f.path(), path, stem, cache, cache_lkp);
        }
    }
}

}
