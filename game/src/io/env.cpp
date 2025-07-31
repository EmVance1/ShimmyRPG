#include "pch.h"
#include <Windows.h>

namespace shmy { namespace env {

static std::string game_env = "res/";

const std::string& get() {
    return game_env;
}

void set(const std::string& env) {
    game_env = env;
}

} }

