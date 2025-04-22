#pragma once
#include <iostream>
#include <unordered_map>
#include <vector>


template<typename T>
std::ostream& operator<<(std::ostream& stream, const std::vector<T>& vec) {
    if (vec.empty()) {
        return stream << "[]";
    } else {
        stream << "[ ";
        for (size_t i = 0; i < vec.size(); i++) {
            stream << vec[i];
            if (i < vec.size() - 1) {
                stream << ", ";
            }
        }
        return stream << " ]";
    }
}

template<>
std::ostream& operator<<(std::ostream& stream, const std::vector<std::string>& vec);


template<typename K, typename V>
std::ostream& operator<<(std::ostream& stream, const std::unordered_map<K, V>& map) {
    if (map.empty()) {
        return stream << "{}";
    } else {
        stream << "{ ";
        size_t i = 0;
        for (const auto& [k, v] : map) {
            stream << k << ": " << v;
            if (i < map.size() - 1) {
                stream << ", ";
            }
        }
        return stream << " }";
    }
}

template<typename V>
std::ostream& operator<<(std::ostream& stream, const std::unordered_map<std::string, V>& map) {
    if (map.empty()) {
        return stream << "{}";
    } else {
        stream << "{ ";
        size_t i = 0;
        for (const auto& [k, v] : map) {
            stream << "\"" << k << "\": " << v;
            if (i < map.size() - 1) {
                stream << ", ";
            }
        }
        return stream << " }";
    }
}

