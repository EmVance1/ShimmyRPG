#pragma once
#include <string>
#include <filesystem>
namespace std { namespace fs = filesystem; }


namespace shmy { namespace env {

const std::fs::path& app_dir();
const std::fs::path& user_dir();
const std::fs::path& pkg();
const std::fs::path& pkg_full();

void init(const std::string& env);
void set_pkg(const std::fs::path& pkg);

} }
