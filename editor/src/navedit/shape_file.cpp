#include "pch.h"
#include "load.h"
#include <iostream>
#include <fstream>
#include <sstream>


std::optional<MeshFile> MeshFile::read_file(const std::filesystem::path& path) {
    auto f = std::ifstream(path);
    if (!f.is_open()) { return {}; }

    auto result = MeshFile{};
    MeshPolygon* poly = nullptr;

    for (std::string line; std::getline(f, line);) {
        if (line == "") {
            continue;
        } else if (line[0] == 's') {
            auto parser = std::stringstream(line);
            char x;
            parser >> x >> result.display_scale;
        } else if (line[0] == 'p') {
            auto parser = std::stringstream(line);
            poly = &result.polys.emplace_back();
            char x;
            parser >> x >> poly->weight;
            poly->is_cutout = (poly->weight < 0.0001f);
        } else if (line[0] == 'v') {
            auto parser = std::stringstream(line);
            char x;
            auto v = nav::Vector2f{};
            parser >> x >> v.x >> v.y;
            poly->poly.push_back(v);
        } else if (line[0] == 'c') {
            auto parser = std::stringstream(line);
            char x;
            auto c = MeshCircle{};
            parser >> x >> c.circle.pos.x >> c.circle.pos.y >> c.circle.radius >> c.weight;
            c.is_cutout = (c.weight < 0.0001f);
            result.circles.push_back(c);
        }
    }

    return result;
}

bool MeshFile::write_file(const std::filesystem::path& path) {
    auto f = std::ofstream(path);
    if (!f.is_open()) { return false; }

    f << "s " << display_scale << "\n\n";
    for (const auto& p : polys) {
        if (p.is_cutout) { f << "p 0.0\n"; } else { f << "p " << p.weight << "\n"; }
        for (const auto& v : p.poly) {
            f << "v " << v.x << " " << v.y << "\n";
        }
        f << "\n";
    }
    for (const auto& c : circles) {
        f << "c " << c.circle.pos.x << " " << c.circle.pos.y << "  " << c.circle.radius << "  ";
        if (c.is_cutout) { f << "0.0\n"; } else { f << c.weight << "\n"; }
    }

    return true;
}

