#pragma once
#include <unordered_map>
#include <vector>
#include <string>
#include <filesystem>


class ModuleCompiler {
private:
    std::unordered_map<std::string, uint32_t> entity_lkp;
    std::unordered_map<std::string, uint32_t> flag_lkp;

private:
    void compile_scene();
    void compile_entity();

public:
    void compile(const std::filesystem::path& in, const std::filesystem::path& out);
};

