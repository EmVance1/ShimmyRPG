#include "pch.h"
#include "../str.h"


namespace shmy { namespace str {

std::string read_to_string(const std::fs::path& filename) {
    auto f = std::ifstream(PATH_NORM(filename), std::ios::binary);
    f.seekg(0, std::ios::end);
    const auto size = (size_t)f.tellg();
    f.seekg(0, std::ios::beg);
    auto buffer = std::make_unique<char[]>(size);
    f.read(buffer.get(), size);
    return std::string(buffer.get(), size);
}

void write_to_file(const std::fs::path& filename, const std::string& content) {
    auto f = std::ofstream(PATH_NORM(filename));
    f.write(content.data(), content.size());
    f.close();
}

} }
