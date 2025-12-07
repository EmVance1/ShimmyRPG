#pragma once
#include <cstdint>
#include <string>


namespace shmy { namespace speech {

struct Graph;
class Edge;
class Line;
class Vertex;

class Edge{
private:
    const Graph* G = nullptr;
    size_t vert = 0;
    size_t index = 0;

    Edge(const Graph* _G, size_t _vert, size_t _index) : G(_G), vert(_vert), index(_index) {}

    using Callback = uint64_t*(*)(const char* key, bool strict);

public:
    Edge() = default;

    Edge next() const;
    const std::string& line() const;
    Vertex next_vertex() const;
    int64_t eval_condition(Callback ctxt) const;
    void    eval_modifiers(Callback ctxt) const;

    friend struct Graph;
    friend class Vertex;
    friend bool operator==(const Edge& lhs, const Edge& rhs);
    friend bool operator!=(const Edge& lhs, const Edge& rhs);
};

bool operator==(const Edge& lhs, const Edge& rhs);
bool operator!=(const Edge& lhs, const Edge& rhs);

class Line {
private:
    const Graph* G = nullptr;
    size_t vert = 0;
    size_t index = 0;

    Line(const Graph* _G, size_t _vert, size_t _index) : G(_G), vert(_vert), index(_index) {}

public:
    Line() = default;

    Line next() const;
    const std::string& line() const;

    friend struct Graph;
    friend class Vertex;
    friend bool operator==(const Line& lhs, const Line& rhs);
    friend bool operator!=(const Line& lhs, const Line& rhs);
};

bool operator==(const Line& lhs, const Line& rhs);
bool operator!=(const Line& lhs, const Line& rhs);

class Vertex{
public:
    enum class Outcome { Edges, Goto, Exit, ExitWith };

private:
    const Graph* G = nullptr;
    size_t index = 0;

    Vertex(const Graph* _G, size_t _index) : G(_G), index(_index) {}

public:
    Vertex() = default;

    const std::string& speaker() const;

    Line first_line() const;
    Line end_line() const;
    size_t line_count() const;

    Edge first_edge() const;
    Edge end_edge() const;
    size_t edge_count() const;

    Outcome outcome() const;
    Vertex goes_to() const;
    const std::string& exits_with() const;

    friend struct Graph;
    friend class Edge;
    friend bool operator==(const Vertex& lhs, const Vertex& rhs);
    friend bool operator!=(const Vertex& lhs, const Vertex& rhs);
};

bool operator==(const Vertex& lhs, const Vertex& rhs);
bool operator!=(const Vertex& lhs, const Vertex& rhs);


} }
