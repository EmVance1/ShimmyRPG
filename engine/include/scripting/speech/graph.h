#pragma once
#include <unordered_map>
#include <variant>
#include <string>
#include <filesystem>
#include "flags.h"
#include "../expr.h"


namespace shmy { namespace speech {

struct Response {
    std::string text;
    std::string edge;
    Expr conditions;
    Expr modifiers;
};
struct Goto { std::string next; bool is_exit; };
using Outcome = std::variant<std::vector<Response>, Goto>;

struct Vertex {
    Expr conditions;
    std::string speaker;
    std::vector<std::string> lines;
    Outcome outcome;
};

using Graph = std::unordered_map<std::string, Vertex>;


Graph load_from_file(const std::filesystem::path& filename);
Graph load_from_string(const std::string& str);
Graph load_from_line(const std::string& speaker, const std::string& line);


bool operator==(const Vertex& a, const Vertex& b);
bool operator==(const Response& a, const Response& b);
bool operator==(const Goto& a, const Goto& b);

} }
