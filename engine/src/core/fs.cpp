#include "pch.h"
#include "core/fs.h"
#include "core/result.h"


namespace shmy { namespace core {

#ifdef _WIN32
#define os_fopen(file, mode) _wfopen(file.c_str(), L##mode)
#else
#define os_fopen(file, mode) fopen(file.c_str(), mode)
#endif


[[nodiscard]] Result<std::string, Error> read_to_string(const std::filesystem::path& filename) {
    FILE* f = os_fopen(filename, "rb");
    if (!f) {
        return Result<std::string, Error>::Err(Error{ Error::FileNotFound });
    }
    fseek(f, 0, SEEK_END);
    const usize size = (usize)ftell(f);
    fseek(f, 0, SEEK_SET);
    auto buf = std::string(size, 'a');
    fread(buf.data(), sizeof(char), size, f);
    fclose(f);
    return Result<std::string, Error>::Ok(std::move(buf));
}

[[nodiscard]] Result<Unit, Error> write(const std::filesystem::path& filename, const std::string& content) {
    FILE* f = os_fopen(filename, "rb");
    if (!f) {
        return Result<Unit, Error>::Err(Error{ Error::FileNotFound });
    }
    fwrite(content.data(), sizeof(char), (usize)content.size(), f);
    fclose(f);
    return Result<Unit, Error>::Ok({});
}


} }

