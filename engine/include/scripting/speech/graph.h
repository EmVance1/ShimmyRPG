#pragma once
#include <filesystem>
#include <string>


namespace shmy { namespace speech {

struct Edge {
    uint32_t condition = 1;
    uint32_t modifiers = 0;
    uint32_t edge;
    uint32_t line;
};

struct Vert {
    uint32_t n_lines = 0;
    uint32_t n_edges = 0;
    uint32_t speaker;
    uint32_t lines;
    uint32_t edges;
};

struct Entry {
    uint32_t condition = 1;
    uint32_t vert;
};

struct Graph {
    constexpr inline static size_t EXIT = UINT32_MAX;
    using Callback = uint64_t*(*)(const char* key, bool strict);

    std::vector<Entry> eps;
    std::vector<Vert> verts;
    std::vector<Edge> edges;
    std::vector<uint8_t> exprs;
    std::vector<std::string> strs;

    static Graph load_from_file(const std::filesystem::path& filename);
    static Graph load_from_string(const std::string& str);
    static Graph create_from_line(const std::string& speaker, const std::string& line);

    int64_t eval_expr(size_t ref, Callback ctxt) const;
};


bool operator==(const Edge& a, const Edge& b);
bool operator!=(const Edge& a, const Edge& b);

bool operator==(const Vert& a, const Vert& b);
bool operator!=(const Vert& a, const Vert& b);

bool operator==(const Entry& a, const Edge& b);
bool operator!=(const Entry& a, const Edge& b);

bool operator==(const Graph& a, const Graph& b);
bool operator!=(const Graph& a, const Graph& b);


} }
