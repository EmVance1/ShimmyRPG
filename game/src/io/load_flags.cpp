#include "pch.h"
#include "load_flags.h"
#include "flags.h"


namespace shmy { namespace flags {

static void load_file(const std::fs::path& filename, const std::fs::path& prefix) {
    auto file = std::ifstream(PATH_NORM(filename));
    auto line = std::string("");
    bool err = false;

    while (std::getline(file, line)) {
        if (line.empty() || (line[0] == '/' && line[1] == '/')) {
            continue;
        }
        auto stream = std::stringstream(line);
        auto key = std::string("");
        auto val = std::string("");
        stream >> key >> val;
        if (key == "true" || key == "false" || key == "inf" || key == "default") {
            std::cerr << "runtime error: flag init error - '" << key << "' is a reserved value and cannot be overriden\n";
            err = true;
            continue;
        }
        if (key.starts_with("rng")) {
            std::cerr << "runtime error: flag init error - 'rng' prefix is reserved for temporary internal flags\n";
            err = true;
            continue;
        }
        if (key.starts_with("once")) {
            std::cerr << "runtime error: flag init error - 'once' prefix is reserved for temporary internal flags\n";
            err = true;
            continue;
        }
        if (prefix != "Global") {
            key = prefix.string() + "_" + key;
        }
        if (val == "true") {
            FlagTable::set_flag(key, 1, false);
        } else if (val == "false") {
            FlagTable::set_flag(key, 0, false);
        } else {
            FlagTable::set_flag(key, std::atoi(val.c_str()), false);
        }
    }

    if (err) { exit(1); }
}


void load_from_dir(const std::fs::path& dir) {
    FlagTable::set_flag("true", 1, false);
    FlagTable::set_flag("false", 0, false);
    FlagTable::set_flag("inf", UINT64_MAX, false);

    for (const auto& f : std::fs::directory_iterator(dir)) {
        load_file(f.path(), f.path().stem());
    }
}

} }
