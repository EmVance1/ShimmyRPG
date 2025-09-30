#include "pch.h"
#include "util/env.h"


namespace shmy { namespace env {

static std::fs::path s_localappdata = "";

static std::fs::path s_app_dir;
static std::fs::path s_user_dir;
static std::fs::path s_pkg;
static std::fs::path s_pkg_full;

static void initlocalappdata();


void init(const std::string& env) {
    initlocalappdata();
    if (env == "DEV") {
        s_app_dir = "./res";
        s_user_dir = ".";
    } else if (env == "CWD") {
        s_app_dir = "./res";
        s_user_dir = ".";
    } else if (env == "LOCALDATA") {
        s_app_dir = "./res";
        s_user_dir = s_localappdata;
    }
}


const std::fs::path& app_dir() {
    return s_app_dir;
}
const std::fs::path& user_dir() {
    return s_user_dir;
}
const std::fs::path& pkg() {
    return s_pkg;
}
const std::fs::path& pkg_full() {
    return s_pkg_full;
}

void set_pkg(const std::fs::path& pkg) {
    s_pkg = pkg;
    s_pkg_full = s_user_dir / pkg;
}

} }


#ifdef _WIN32
#include <Windows.h>
#include <userenv.h>
#include <shlobj.h>

namespace shmy { namespace env {

static void initlocalappdata() {
    WCHAR* p = nullptr;
    SHGetKnownFolderPath(FOLDERID_LocalAppData, 0, nullptr, &p);
    s_localappdata = std::fs::path(p) / "shimmy";
    CoTaskMemFree(p);
}

} }

#else
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>

namespace shmy { namespace env {

static void initlocalappdata() {
    struct passwd* pw = getpwuid(getuid());
    const char* homedir = pw->pw_dir;
    s_localappdata = std::fs::path(homedir) / ".config" / "shimmy";
}

} }

#endif

