#pragma once
#include <navmesh/lib.h>
#include <optional>
#include <vector>


using Polygon = std::vector<nav::Vector2f>;

struct MeshPolygon {
    Polygon poly;
    float weight;
    bool is_cutout;
};

struct MeshCircle {
    nav::FloatCircle circle;
    float weight;
    bool is_cutout;
};

struct MeshFile {
    std::vector<MeshPolygon> polys;
    std::vector<MeshCircle> circles;
    float display_scale;

    static std::optional<MeshFile> read_file(const std::filesystem::path& path);
    bool write_file(const std::filesystem::path& path);
};

