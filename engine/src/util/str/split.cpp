#include "pch.h"
#include "util/str.h"


namespace shmy { namespace str {

std::vector<std::string> split(const std::string& str, char delimiter) {
    auto result = std::vector<std::string>();
    size_t last = 0;
    bool dont = false;

    for (size_t i = 0; i < str.size(); i++) {
        if (str[i] == delimiter) {
            result.push_back(str.substr(last, i - last));
            if (i + 1 < str.size()) {
                last = i + 1;
            } else {
                dont = true;
            }
        }
    }

    if (!dont) {
        result.push_back(str.substr(last, str.size() - last));
    }

    return result;
}

std::vector<std::string> tok(const std::string& str, char delimiter) {
    auto result = std::vector<std::string>();
    size_t last = 0;
    bool dont = false;

    for (size_t i = 0; i < str.size(); i++) {
        if (str[i] == delimiter) {
            if (last != i - last) {
                result.push_back(str.substr(last, i - last));
            }
            if (i + 1 < str.size()) {
                last = i + 1;
            } else {
                dont = true;
            }
        }
    }

    if (!dont) {
        result.push_back(str.substr(last, str.size() - last));
    }

    return result;
}


std::string trim(const std::string& str) {
    size_t begin = 0;
    while (begin < str.size() && std::isspace(str[begin])) {
        begin++;
    }
    size_t len = str.size();
    while (len > 0 && std::isspace(str[len - 1])) {
        len--;
    }
    if (begin == str.size() || len == 0) {
        return "";
    } else {
        return str.substr(begin, len - begin);
    }
}

} }
