#pragma once
#include "result.h"
#include <filesystem>


namespace shmy { namespace core {

struct Error {
    enum Kind { FileNotFound } kind;

    const char* to_string() const {
        switch (kind) {
            case FileNotFound:
                return "file not found";
        }
        return "";
    }
};

[[nodiscard]] Result<std::string, Error> read_to_string(const std::filesystem::path& filename);
[[nodiscard]] Result<Unit, Error> write(const std::filesystem::path& filename, const std::string& content);

} }

