#include "pch.h"
#include "../str.h"


std::string read_to_string(const std::string& filename) {
    auto f = std::ifstream(filename);

    f.seekg(0, std::ios::end);
    const auto size = (size_t)f.tellg();
    f.seekg(0, std::ios::beg);

    auto buf = std::string("");
    buf.resize(size);
    f.read(buf.data(), size);
    f.close();
    return buf;
}

void write_to_file(const std::string& filename, const std::string& content) {
    auto f = std::ofstream(filename);
    f.write(content.data(), content.size());
    f.close();
}

