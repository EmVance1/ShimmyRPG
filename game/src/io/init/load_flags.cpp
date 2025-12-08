#include "pch.h"
#include "core/split.h"
#include "load_flags.h"
#include "flags.h"


namespace shmy { namespace flags {

std::pair<std::string, std::string> split_kv(const std::string& str, const std::fs::path& filename, size_t line, bool& err) {
    for (int i = 0; i < str.size(); i++) {
        if (str[i] == '=') {
            const auto key = str.substr(0, i);
            const auto val = str.substr(i+1);
            return { core::trim(key), core::trim(val) };
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
        if (core::trim(str).empty() || (str[0] == '#')) {
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
            FlagTable::set(key, std::atoi(val.c_str()), false);
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


void load_from_dir(const std::fs::path& dir) {
    load_dir(dir, "");
}

} }
