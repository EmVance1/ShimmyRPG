#include "pch.h"
#include <string>
#include <sstream>
#include "util/random.h"


namespace shmy { namespace core {

std::string generate_uuid_v4() {
    std::stringstream ss;
    ss << std::hex;
    for (int i = 0; i < 8; i++) {
        ss << Random::integer(0, 15);
    }
    ss << "-";
    for (int i = 0; i < 4; i++) {
        ss << Random::integer(0, 15);
    }
    ss << "-4";
    for (int i = 0; i < 3; i++) {
        ss << Random::integer(0, 15);
    }
    ss << "-" << Random::integer(8, 11);
    for (int i = 0; i < 3; i++) {
        ss << Random::integer(0, 15);
    }
    ss << "-";
    for (int i = 0; i < 12; i++) {
        ss << Random::integer(0, 15);
    };
    return ss.str();
}

} }
