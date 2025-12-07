#include "pch.h"
#include "scripting/speech/graph.h"
#include "scripting/expr.h"
#include "core/fs.h"


namespace shmy { namespace speech {


Graph Graph::load_from_file(const std::fs::path& filename) {
    return Graph::load_from_string(core::read_to_string(filename).unwrap());
}

Graph Graph::create_from_line(const std::string& speaker, const std::string& line) {
    return Graph{
        .eps={ _Entry{
            .condition=1,
            .vert=0,
        } },
        .verts={ _Vert{
            .n_lines=1,
            .n_edges=Graph::EXIT,
            .speaker=0,
            .lines=1,
            .edges=0,
        } },
        .edges={},
        .exprs={ Expr::CFalse, Expr::CTrue },
        .strs={ speaker, line },
    };
}

int64_t Graph::eval_expr(size_t ref, Callback ctxt) const {
    switch (ref) {
    case 0: return 0;
    case 1: return 1;
    default: return Expr::evaluate(&exprs[ref], strs, ctxt);
    }
}


bool operator==(const _Edge& a, const _Edge& b) {
    return memcmp(&a, &b, sizeof(_Edge)) == 0;
}
bool operator!=(const _Edge& a, const _Edge& b) {
    return !(a == b);
}

bool operator==(const _Vert& a, const _Vert& b) {
    return memcmp(&a, &b, sizeof(_Vert)) == 0;
}
bool operator!=(const _Vert& a, const _Vert& b) {
    return !(a == b);
}

bool operator==(const _Entry& a, const _Entry& b) {
    return memcmp(&a, &b, sizeof(_Entry)) == 0;
}
bool operator!=(const _Entry& a, const _Entry& b) {
    return !(a == b);
}

bool operator==(const Graph& a, const Graph& b) {
    return a.eps == b.eps &&
           a.verts == b.verts &&
           a.edges == b.edges &&
           a.exprs == b.exprs &&
           a.strs == b.strs;
}
bool operator!=(const Graph& a, const Graph& b) {
    return !(a == b);
}


} }
