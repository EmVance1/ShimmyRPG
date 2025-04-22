#include "pch.h"
#include "io.h"


template<>
std::ostream& operator<<(std::ostream& stream, const std::vector<std::string>& vec) {
    if (vec.empty()) {
        return stream << "[]";
    } else {
        stream << "[ ";
        for (size_t i = 0; i < vec.size(); i++) {
            stream << "\"" << vec[i] << "\"";
            if (i < vec.size() - 1) {
                stream << ", ";
            }
        }
        return stream << " ]";
    }
}

