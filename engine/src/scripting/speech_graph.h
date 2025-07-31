#pragma once
#include <unordered_map>
#include <variant>
#include <string>
#include "flags.h"
#include "flag_expr.h"


namespace shmy { namespace speech {

struct Response {
    FlagExpr conditions;
    std::string text;
    std::string edge;
    std::unordered_map<std::string, FlagModifier> flags;
};
struct Goto { std::string vertex; };
struct Exit {};
struct ExitInto { std::string script; };

using Outcome = std::variant<std::vector<Response>, Goto, Exit, ExitInto>;

struct Vertex {
    FlagExpr conditions;
    std::string speaker;
    std::vector<std::string> lines;
    Outcome outcome;
};

using Graph = std::unordered_map<std::string, Vertex>;


Graph load_from_file(const std::string& filename);
Graph load_from_string(const std::string& str);
Graph load_from_line(const std::string& speaker, const std::string& line);


bool operator==(const Vertex& a, const Vertex& b);
bool operator==(const Response& a, const Response& b);
bool operator==(const Goto& a, const Goto& b);
bool operator==(const Exit& a, const Exit& b);
bool operator==(const ExitInto& a, const ExitInto& b);

} }
