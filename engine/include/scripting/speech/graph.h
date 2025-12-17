#pragma once
#include <filesystem>
#include <string>
#include "iter.h"


namespace shmy { namespace speech {

struct _Edge {
    uint32_t condition = 1;
    uint32_t outcome = 0;
    uint32_t edge;
    uint32_t line;
};

struct _Vert {
    uint32_t n_lines = 0;
    uint32_t n_edges = 0;
    uint32_t speaker;
    uint32_t lines;
    uint32_t edges;
};

struct _Entry {
    uint32_t condition = 1;
    uint32_t vert;
};

struct Graph {
    constexpr inline static size_t EXIT      = UINT32_MAX;
    constexpr inline static size_t EXIT_WITH = UINT32_MAX-1;
    using Callback = uint64_t*(*)(const char* key, bool strict);

    std::vector<_Entry> eps;
    std::vector<_Vert> verts;
    std::vector<_Edge> edges;
    std::vector<uint8_t> exprs;
    std::vector<std::string> strs;

    static Graph load_from_file(const std::filesystem::path& filename);
    static Graph load_from_string(const std::string& str);
    static Graph create_from_line(const std::string& speaker, const std::string& line);

    int64_t eval_expr(size_t ref, Callback ctxt) const;

    Vertex first_entry() const { return { this, eps[0].vert }; }

    Vertex nth_vertex(size_t n) const { return { this, n }; }
    size_t vertex_count() const { return verts.size(); }
};

bool operator==(const _Edge& a, const _Edge& b);
bool operator!=(const _Edge& a, const _Edge& b);

bool operator==(const _Vert& a, const _Vert& b);
bool operator!=(const _Vert& a, const _Vert& b);

bool operator==(const _Entry& a, const _Entry& b);
bool operator!=(const _Entry& a, const _Entry& b);

bool operator==(const Graph& a, const Graph& b);
bool operator!=(const Graph& a, const Graph& b);


} }
