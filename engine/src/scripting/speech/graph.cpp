#include "pch.h"
#include "graph.h"
#include "../lexer.h"
#include "util/str.h"


namespace shmy { namespace speech {

Graph parse_graph(detail::Lexer&& lexer);

Graph load_from_file(const std::fs::path& filename) {
    const auto src = str::read_to_string(filename);
    auto lexer = detail::Lexer(src);
    try {
        return parse_graph(std::move(lexer));
    } catch (const std::exception& e) {
        std::cout << e.what() << "\n";
        return {};
    }
}

Graph load_from_line(const std::string& speaker, const std::string& line) {
    auto res = Graph();
    res.emplace("entry0", Vertex{
        FlagExpr::True(),
        speaker,
        { line },
        Exit{}
    });
    return res;
}


bool operator==(const Vertex& a, const Vertex& b) {
    return a.speaker == b.speaker &&
           a.lines == b.lines &&
           a.outcome == b.outcome;
}

bool operator==(const Response& a, const Response& b) {
    return a.conditions == b.conditions &&
           a.text == b.text &&
           a.edge == b.edge &&
           a.flags == b.flags;
}

bool operator==(const Goto& a, const Goto& b) {
    return a.vertex == b.vertex;
}

bool operator==(const Exit&, const Exit&) {
    return true;
}

bool operator==(const ExitInto& a, const ExitInto& b) {
    return a.script == b.script;
}

} }
