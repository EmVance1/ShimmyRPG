#pragma once
#include "scripting/speech/lexer.h"
#include "scripting/speech/graph.h"
#include <unordered_map>


namespace shmy { namespace speech { namespace detail {


struct ParseContext {
    struct VertPair { uint32_t idx; bool init; };

    Lexer lexer;
    Graph* graph;
    bool set_strict = true;
    bool link_audio = false;
    std::unordered_map<std::string, uint32_t> s_LUT = {};
    std::unordered_map<std::string, VertPair> v_LUT = {};

    std::string unwrap_next(TokenType expect, const std::string& error);
    std::string unwrap_next(uint32_t expect, const std::string& error);

    uint32_t push_next_expr();
    uint32_t push_str(std::string&& val, bool dedup = true);
    uint32_t push_vertex(const std::string& name, bool init = false);
};


} } }
