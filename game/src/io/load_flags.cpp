#include "pch.h"
#include "load_flags.h"
#include "flags.h"


static void load_file(const std::string& filename, const std::string& prefix) {
    auto file = std::ifstream(filename);
    auto line = std::string("");
    bool err = false;

    while (std::getline(file, line)) {
        if (line.empty() || (line[0] == '/' && line[1] == '/')) {
            continue;
        }
        auto stream = std::stringstream(line);
        auto f = std::string("");
        auto v = std::string("");
        stream >> f >> v;
        if (f == "true" || f == "false" || f == "inf" || f == "default") {
            std::cerr << "runtime error: flag init error - '" << f << "' is a reserved value and cannot be overriden\n";
            err = true;
            continue;
        }
        if (f.starts_with("rng")) {
            std::cerr << "runtime error: flag init error - 'rng' prefix is reserved for temporary internal flags\n";
            err = true;
            continue;
        }
        if (f.starts_with("once")) {
            std::cerr << "runtime error: flag init error - 'once' prefix is reserved for temporary internal flags\n";
            err = true;
            continue;
        }
        if (prefix != "Global") {
            f = prefix + "_" + f;
        }
        if (v == "true") {
            FlagTable::set_flag(f, 1, false);
        } else if (v == "false") {
            FlagTable::set_flag(f, 0, false);
        } else {
            FlagTable::set_flag(f, std::atoi(v.c_str()), false);
        }
    }

    if (err) { exit(1); }
}


void load_flags(const std::string& dir) {
    FlagTable::set_flag("true", 1, false);
    FlagTable::set_flag("false", 0, false);
    FlagTable::set_flag("inf", UINT32_MAX, false);
    FlagTable::set_flag("default", 1, false);

    for (const auto& f : std::fs::directory_iterator(dir)) {
        load_file(f.path().generic_string(), f.path().stem().generic_string());
    }
}

