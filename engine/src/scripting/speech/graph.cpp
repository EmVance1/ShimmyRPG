#include "pch.h"
#include "scripting/speech/graph.h"
#include "scripting/lexer.h"
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
        Expr::True(),
        speaker,
        { line },
        Goto{ "", true }
    });
    return res;
}


bool operator==(const Vertex& a, const Vertex& b) {
    return a.speaker == b.speaker &&
           a.lines == b.lines &&
           a.outcome == b.outcome;
}

bool operator==(const Response& a, const Response& b) {
    return a.text == b.text &&
           a.edge == b.edge &&
           a.conditions == b.conditions &&
           a.modifiers == b.modifiers;
}

bool operator==(const Goto& a, const Goto& b) {
    return a.next == b.next &&
           a.is_exit == b.is_exit;
}

} }
