#include "pch.h"
#include "util/split.h"


namespace shmy::core {

#define SOME(x) Option<decltype(x)>::Some(std::move(x))
#define NONE(T) Option<T>::None()


std::vector<std::string> split(const std::string& val, char pat) {
    auto result = std::vector<std::string>();
    size_t last = 0;
    bool dont = false;

    for (size_t i = 0; i < val.size(); i++) {
        if (val[i] == pat) {
            result.push_back(val.substr(last, i - last));
            if (i + 1 < val.size()) {
                last = i + 1;
            } else {
                dont = true;
            }
        }
    }

    if (!dont) {
        result.push_back(val.substr(last, val.size() - last));
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

}

