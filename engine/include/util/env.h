#pragma once
#include <filesystem>
namespace std { namespace fs = filesystem; }


namespace shmy { namespace env {

const std::fs::path& app_dir();
const std::fs::path& user_dir();
const std::fs::path& pkg();
const std::fs::path& pkg_full();
std::vector<std::fs::path> pkg_list();

enum class Env { CWD, LocalAppData };
void init(Env env);
void set_pkg(const std::fs::path& pkg);

} }
