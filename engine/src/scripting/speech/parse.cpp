#include "pch.h"
#include "scripting/speech/graph.h"
#include "scripting/speech/lexer.h"
#include "scripting/speech/ctxt.h"
#include "scripting/expr.h"


namespace shmy { namespace speech { namespace detail {

#define PARSE_ERROR(tok, msg) std::runtime_error(std::string(msg) + \
        "token '" + tok->val + "' at " + std::to_string(tok->row) + ":" + std::to_string(tok->col))

static Graph parse_graph(Lexer&& lexer);
static bool parse_vertex(ParseContext& ctx);
static void parse_outcome(ParseContext& ctx, size_t vert);
static void parse_modifiers(ParseContext& ctx, Edge& edge);
static void parse_pragma(ParseContext& ctx, const std::string& pragma);

}


Graph Graph::load_from_string(const std::string& src) {
    try {
        return parse_graph(detail::Lexer(src));
    } catch (const std::exception& e) {
        std::cerr << "parse shmy error - " << e.what() << "\n";
        exit(1);
    }
}


namespace detail {

static Graph parse_graph(Lexer&& lexer) {
    auto graph = Graph();
    // reserved indicies
    graph.exprs.push_back(Expr::IEndOf); // 0 = false / no modifiers
    graph.exprs.push_back(Expr::IEndOf); // 1 = true
    auto ctx = ParseContext{ std::move(lexer), &graph };
    ctx.v_LUT["exit"] = { Graph::EXIT, true };
    while (parse_vertex(ctx));
    for (const auto& [k, v] : ctx.v_LUT) {
        if (!v.init) {
            throw std::runtime_error("reference to undefined vertex '" + k + "'");
        }
    }
    return graph;
}


static bool parse_vertex(ParseContext& ctx) {
    if (!ctx.lexer.peek().has_value()) return false;
    while (ctx.lexer.peek()->type == TokenType::Pragma) {
        parse_pragma(ctx, ctx.lexer.next()->val);
        if (!ctx.lexer.peek().has_value()) return false;
    }

    const auto key = ctx.unwrap_next((uint32_t)TokenType::Identifier|(uint32_t)TokenType::IntLiteral, "token unwrapped to wrong type - ");
    size_t current = Graph::EXIT;
    if (key == "entry") {
        ctx.unwrap_next(TokenType::OpenParen, "'entry' verts must have a condition or 'default' - ");
        auto& tmp = ctx.graph->eps.emplace_back();
        tmp.condition = ctx.push_next_expr();
        tmp.vert = (uint32_t)ctx.graph->verts.size();
        current = ctx.graph->verts.size();
        ctx.graph->verts.emplace_back();
        ctx.unwrap_next(TokenType::CloseParen, "shouldn't be possible??? - ");

    } else {
        current = ctx.push_vertex(key, true);
    }

    auto& cur = ctx.graph->verts[current];
    ctx.unwrap_next(TokenType::EqualTo,     "expected '=' after vertex declaration - ");
    cur.speaker = ctx.push_str(ctx.unwrap_next(TokenType::Identifier,  "vertex definition must start with speaker ID - "));
    ctx.unwrap_next(TokenType::Colon,       "expected ':' after speaker ID - ");
    ctx.unwrap_next(TokenType::OpenBracket, "expected string array here - ");

    while (true) {
        switch (ctx.lexer.peek()->type) {
        case TokenType::CloseBracket:
            ctx.lexer.next();
            ctx.unwrap_next(TokenType::Arrow, "expected '=>' to denote vertex outcome - ");
            parse_outcome(ctx, current);
            return true;

        case TokenType::StringLiteral: {
            const auto idx = ctx.push_str(std::move(ctx.lexer.next()->val), false);
            if (cur.n_lines++ == 0) {
                cur.lines = idx;
            }
            break; }

        default:
            throw PARSE_ERROR(ctx.lexer.peek(), "expexted <string> or end of array - ");
        };

        switch (ctx.lexer.peek()->type) {
        case TokenType::CloseBracket: break;
        case TokenType::Comma: ctx.lexer.next(); break;
        default:
            throw PARSE_ERROR(ctx.lexer.peek(), "expexted ',' or end of array  - ");
        }
    }

    return true;
}


static void parse_outcome(ParseContext& ctx, size_t vert) {
    auto next = ctx.lexer.next();
    switch (next->type) {
    case TokenType::Identifier:
        if (next->val == "exit") {
            ctx.graph->verts[vert].n_edges = Graph::EXIT;
            ctx.graph->verts[vert].edges = 0;

        } else if (next->val == "exit_into") {
            ctx.unwrap_next(TokenType::OpenBrace,  "expected '{' after 'exit_into' special vertex - ");
            ctx.graph->verts[vert].n_edges = Graph::EXIT;
            ctx.graph->verts[vert].edges = ctx.push_str(ctx.unwrap_next(TokenType::StringLiteral, "'exit_into' block must contain a script name - "));
            ctx.unwrap_next(TokenType::CloseBrace, "expected '}' to end 'exit_into' block - ");

        } else {
            ctx.graph->verts[vert].n_edges = 0;
            ctx.graph->verts[vert].edges = ctx.push_vertex(next->val);
        }
        return;

    case TokenType::OpenBrace:
        break;

    default:
        throw PARSE_ERROR(next, "token unwrapped to wrong type - ");
    }

    auto& edges = ctx.graph->edges;
    ctx.graph->verts[vert].edges = (uint32_t)edges.size();
    while (true) {
        next = ctx.lexer.next();
        switch (next->type) {
        case TokenType::OpenParen: {
            auto& temp = edges.emplace_back();
            temp.condition = ctx.push_next_expr();
            ctx.unwrap_next(TokenType::CloseParen, "shouldn't be possible??? - ");
            temp.line = ctx.push_str(ctx.unwrap_next(TokenType::StringLiteral, "token unwrapped to wrong type - "));
            ctx.graph->verts[vert].n_edges++;
            break; }

        case TokenType::StringLiteral: {
            auto& temp = edges.emplace_back();
            temp.condition = 1;
            temp.line = ctx.push_str(std::move(next->val));
            ctx.graph->verts[vert].n_edges++;
            break; }

        case TokenType::CloseBrace:
            return;

        default:
            throw PARSE_ERROR(next, "token unwrapped to wrong type - ");
        };

        auto& resp = edges.back();

        ctx.unwrap_next(TokenType::Arrow, "token unwrapped to wrong type - ");
        const auto edge = ctx.unwrap_next((uint32_t)TokenType::Identifier|(uint32_t)TokenType::IntLiteral, "token unwrapped to wrong type - ");
        resp.edge = ctx.push_vertex(edge);

        switch (ctx.lexer.next()->type) {
        case TokenType::Comma:
            resp.modifiers = 0;
            break;

        case TokenType::OpenBrace:
            parse_modifiers(ctx, resp);
            switch (ctx.lexer.next()->type) {
            case TokenType::CloseBrace: return;
            case TokenType::Comma: break;
            default:
                throw PARSE_ERROR(next, "token unwrapped to wrong type - ");
            }
            break;

        default:
            throw PARSE_ERROR(next, "token unwrapped to wrong type - ");
        }
    }
}


static void parse_modifiers(ParseContext& ctx, Edge& edge) {
    auto& bytecode = ctx.graph->exprs;
    edge.modifiers = (uint32_t)bytecode.size();

    while (true) {
        size_t key = 0;
        auto next = ctx.lexer.next();
        switch (next->type) {
        case TokenType::CloseBrace:
            if (edge.modifiers != bytecode.size()) {
                bytecode.push_back(Expr::IEndOf);
            }
            return;

        case TokenType::Identifier:
            if (edge.modifiers == bytecode.size() && ctx.set_strict) {
                bytecode.push_back(Expr::IStrict);
            }
            key = ctx.push_str(std::move(next->val));
            break;

        default:
            throw PARSE_ERROR(next, "expected modifier or end of list - ");
        }
        ctx.unwrap_next(TokenType::Colon, "expected colon to denote modification - ");

        {
        bytecode.push_back(Expr::IPushK);
        const auto idx = bytecode.size();
        bytecode.resize(idx + sizeof(size_t));
        memcpy(bytecode.data() + idx, &key, sizeof(size_t));
        }

        bytecode.push_back(Expr::IPushC);
        const auto idx = bytecode.size();
        bytecode.resize(idx + sizeof(int64_t));

        next = ctx.lexer.next();
        switch (next->type) {
        case TokenType::Add: {
            const auto val = ctx.unwrap_next(TokenType::IntLiteral, "flag modifier must be integer - ");
            const auto n = std::atoll(val.c_str());
            memcpy(bytecode.data() + idx, &n, sizeof(int64_t));
            bytecode.push_back(Expr::IAddV);
            break; }

        case TokenType::Sub: {
            const auto val = ctx.unwrap_next(TokenType::IntLiteral, "flag modifier must be integer - ");
            const auto n = -std::atoll(val.c_str());
            memcpy(bytecode.data() + idx, &n, sizeof(int64_t));
            bytecode.push_back(Expr::IAddV);
            break; }

        case TokenType::EqualTo: {
            const auto val = ctx.unwrap_next(TokenType::IntLiteral, "flag modifier must be integer - ");
            const auto n = std::atoll(val.c_str());
            memcpy(bytecode.data() + idx, &n, sizeof(int64_t));
            bytecode.push_back(Expr::ISetV);
            break; }

        default:
            throw PARSE_ERROR(next, "expected '+'|'-'|'=' to specify modifier - ");
        }

        switch (ctx.lexer.peek()->type) {
        case TokenType::CloseBrace: break;
        case TokenType::Comma: ctx.lexer.next(); break;
        default:
            throw PARSE_ERROR(next, "token unwrapped to wrong type - ");
        }
    }
}


static void parse_pragma(ParseContext& ctx, const std::string& pragma) {
    if (pragma == "!SET_OR_CREATE") {
        ctx.set_strict = false;
    } else if (pragma == "!SET_STRICT") {
        ctx.set_strict = true;
    } else if (pragma == "!LINK_AUDIO") {
        ctx.link_audio = true;
    }
}

} } }
