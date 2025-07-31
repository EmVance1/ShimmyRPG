#pragma once
#include <string>

namespace shmy { namespace env {

const std::string& get();

void set(const std::string& env);

} }

