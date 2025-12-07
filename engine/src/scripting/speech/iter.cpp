#include "pch.h"
#include "scripting/speech/iter.h"
#include "scripting/speech/graph.h"
#include "scripting/expr.h"


namespace shmy { namespace speech {


Edge Edge::next() const {
    return {
        G,
        vert,
        index + 1,
    };
}

const std::string& Edge::line() const {
    return G->strs[G->edges[G->verts[vert].edges + index].line];
}

Vertex Edge::next_vertex() const {
    return { G, G->edges[G->verts[vert].edges + index].edge };
}

int64_t Edge::eval_condition(Callback ctxt) const {
    const auto n = G->edges[G->verts[vert].edges + index].condition;
    if (n == 0) return 0;
    if (n == 1) return 1;
    const auto bc = &G->exprs[n];
    return Expr::evaluate(bc, G->strs, ctxt);
}

void Edge::eval_modifiers(Expr::Callback ctxt) const {
    const auto n = G->edges[G->verts[vert].edges + index].modifiers;
    if (n == 0) return;
    const auto bc = &G->exprs[n];
    Expr::evaluate(bc, G->strs, ctxt);
}

bool operator==(const Edge& lhs, const Edge& rhs) {
    return lhs.G == rhs.G && lhs.vert == rhs.vert && lhs.index == rhs.index;
}
bool operator!=(const Edge& lhs, const Edge& rhs) {
    return !(lhs == rhs);
}


Line Line::next() const {
    return {
        G,
        vert,
        index + 1,
    };
}

const std::string& Line::line() const {
    return G->strs[G->verts[vert].lines + index];
}

bool operator==(const Line& lhs, const Line& rhs) {
    return lhs.G == rhs.G && lhs.vert == rhs.vert && lhs.index == rhs.index;
}
bool operator!=(const Line& lhs, const Line& rhs) {
    return !(lhs == rhs);
}


const std::string& Vertex::speaker() const {
    return G->strs[G->verts[index].speaker];
}

Line Vertex::first_line() const {
    return { G, index, 0 };
}

Line Vertex::end_line() const {
    return { G, index, line_count() };
}

size_t Vertex::line_count() const {
    return G->verts[index].n_lines;
}

Edge Vertex::first_edge() const {
    return { G, index, 0 };
}

Edge Vertex::end_edge() const {
    return { G, index, edge_count() };
}

size_t Vertex::edge_count() const {
    const auto n = G->verts[index].n_edges;
    return n == Graph::EXIT ? 0 : n;
}

Vertex::Outcome Vertex::outcome() const {
    const auto n = G->verts[index].n_edges;
    if (n == 0) {
        return Outcome::Goto;
    } else if (n == Graph::EXIT) {
        if (G->verts[index].edges == 0) {
            return Outcome::Exit;
        } else {
            return Outcome::ExitWith;
        }
    } else {
        return Outcome::Edges;
    }
}

Vertex Vertex::goes_to() const {
    return { G, G->verts[index].edges };
}

const std::string& Vertex::exits_with() const {
    return G->strs[G->verts[index].edges];
}

bool operator==(const Vertex& lhs, const Vertex& rhs) {
    return lhs.G == rhs.G && lhs.index == rhs.index;
}
bool operator!=(const Vertex& lhs, const Vertex& rhs) {
    return !(lhs == rhs);
}


} }
