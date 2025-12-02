#pragma once
#include <filesystem>
#include <cstdint>


struct Settings {
    static uint32_t x_resolution;
    static uint32_t y_resolution;
    static uint32_t bitsperpixel;
    static uint32_t antialiasing;
    static bool     enable_vsync;

    static void init(const std::filesystem::path& path);
};

