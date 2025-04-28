#include "pch.h"
#include "load_flags.h"


static void load_file(const std::string& filename, const std::string& prefix) {
    auto file = std::ifstream(filename);
    auto line = std::string("");
    while (std::getline(file, line)) {
        if (line.empty() || (line[0] == '/' && line[1] == '/')) {
            continue;
        }
        auto stream = std::stringstream(line);
        auto f = std::string("");
        auto v = std::string("");
        stream >> f >> v;
        if (f == "true" || f == "false" || f == "inf" || f == "default") {
            std::cerr << "'" << f << "' is a reserved value and cannot be overriden\n";
            continue;
        }
        if (prefix != "Global") {
            f = prefix + "_" + f;
        }
        if (v == "true") {
            FlagTable::set_flag(f, 1);
        } else if (v == "false") {
            FlagTable::set_flag(f, 0);
        } else {
            FlagTable::set_flag(f, std::atoi(v.c_str()));
        }
    }
}


void load_flags() {
    FlagTable::set_flag("default", 1);
    FlagTable::set_flag("true", 1);
    FlagTable::set_flag("false", 0);
    FlagTable::set_flag("inf", UINT32_MAX);

    for (const auto& f : std::fs::directory_iterator("res/flags/")) {
        load_file(f.path().generic_string(), f.path().stem().generic_string());
    }
}

