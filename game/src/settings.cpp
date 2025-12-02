#include "pch.h"
#include "settings.h"


uint32_t Settings::x_resolution;
uint32_t Settings::y_resolution;
uint32_t Settings::bitsperpixel;
uint32_t Settings::antialiasing;
bool     Settings::enable_vsync;


void Settings::init(const std::filesystem::path& path) {
    const auto modes = sf::VideoMode::getFullscreenModes();

    Settings::x_resolution = modes[0].size.x;
    Settings::y_resolution = modes[0].size.y;
    Settings::bitsperpixel = modes[0].bitsPerPixel;
    Settings::antialiasing = 0;
    Settings::enable_vsync = false;

    auto fi = std::ifstream(path);
    if (fi.is_open()) {
        fi.read((char*)&Settings::x_resolution, sizeof(Settings::x_resolution));
        fi.read((char*)&Settings::y_resolution, sizeof(Settings::y_resolution));
        fi.read((char*)&Settings::bitsperpixel, sizeof(Settings::bitsperpixel));
        fi.read((char*)&Settings::antialiasing, sizeof(Settings::antialiasing));
        fi.read((char*)&Settings::enable_vsync, sizeof(Settings::enable_vsync));
        fi.close();
    } else {
        auto fo = std::ofstream(path);
        fo.write((char*)&Settings::x_resolution, sizeof(Settings::x_resolution));
        fo.write((char*)&Settings::y_resolution, sizeof(Settings::y_resolution));
        fo.write((char*)&Settings::bitsperpixel, sizeof(Settings::bitsperpixel));
        fo.write((char*)&Settings::antialiasing, sizeof(Settings::antialiasing));
        fo.write((char*)&Settings::enable_vsync, sizeof(Settings::enable_vsync));
        fo.close();
    }
}

