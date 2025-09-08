#include "pch.h"
#include "../str.h"


namespace shmy { namespace str {

std::string read_to_string(const std::fs::path& filename) {
    auto f = std::ifstream(filename, std::ios::binary);
    f.seekg(0, std::ios::end);
    const auto size = f.tellg();
    if (size < 0) return "";
    f.seekg(0, std::ios::beg);
    auto buffer = std::string(size, '\0');
#ifdef VANGO_DEBUG
    if (!f.read(buffer.data(), size)) {
        std::cout << "streaming error - file " << filename << " could not be read\n";
        exit(1);
    }
    if (!utf8::is_valid(buffer)) {
        std::cout << "encoding error - file " << filename << " was not valid utf-8\n";
        exit(1);
    }
#else
    f.read(buffer.data(), size);
#endif
    return buffer;
}

void write_to_file(const std::fs::path& filename, const std::string& content) {
    auto f = std::ofstream(PATH_NORM(filename));
    f.write(content.data(), content.size());
    f.close();
}

} }
