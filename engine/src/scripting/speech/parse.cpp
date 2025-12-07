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
static void parse_modifiers(ParseContext& ctx, _Edge& edge);
static void parse_pragma(ParseContext& ctx, std::optional<Token> tok);

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
            throw std::runtime_error("reference to undefined node '" + k + "'");
        }
    }
    return graph;
}


static bool parse_vertex(ParseContext& ctx) {
    if (!ctx.lexer.peek().has_value()) return false;
    while (ctx.lexer.peek()->type == TokenType::Pragma) {
        parse_pragma(ctx, ctx.lexer.next());
        if (!ctx.lexer.peek().has_value()) return false;
    }

    const auto key = ctx.unwrap_next((uint32_t)TokenType::Identifier|(uint32_t)TokenType::IntLiteral, "expected <identifier> node id - ");
    size_t current = Graph::EXIT;
    if (key == "entry") {
        ctx.unwrap_next(TokenType::OpenParen, "'entry' nodes must have a condition or 'default' - ");
        auto& tmp = ctx.graph->eps.emplace_back();
        tmp.condition = ctx.push_next_expr();
        tmp.vert = (uint32_t)ctx.graph->verts.size();
        current = ctx.graph->verts.size();
        ctx.graph->verts.emplace_back();
        ctx.unwrap_next(TokenType::CloseParen, "ENGINE ERROR UNREACHABLE - ");

    } else {
        current = ctx.push_vertex(key, true);
    }

    auto& cur = ctx.graph->verts[current];
    ctx.unwrap_next(TokenType::EqualTo,     "expected '=' after node declaration - ");
    cur.speaker = ctx.push_str(ctx.unwrap_next(TokenType::Identifier,  "node definition must start with speaker ID - "));
    ctx.unwrap_next(TokenType::Colon,       "expected ':' after speaker ID - ");
    ctx.unwrap_next(TokenType::OpenBracket, "expected string array - ");

    while (true) {
        switch (ctx.lexer.peek()->type) {
        case TokenType::CloseBracket:
            ctx.lexer.next();
            ctx.unwrap_next(TokenType::Arrow, "expected '=>' to denote node outcome - ");
            parse_outcome(ctx, current);
            return true;

        case TokenType::StringLiteral: {
            const auto idx = ctx.push_str(std::move(ctx.lexer.next()->val), false);
            if (cur.n_lines++ == 0) {
                cur.lines = idx;
            }
            break; }

        default:
            throw PARSE_ERROR(ctx.lexer.peek(), "strings in lines array must be ',' separated - ");
        };

        switch (ctx.lexer.peek()->type) {
        case TokenType::CloseBracket: break;
        case TokenType::Comma: ctx.lexer.next(); break;
        default:
            throw PARSE_ERROR(ctx.lexer.peek(), "strings in lines array must be ',' separated - ");
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
            ctx.unwrap_next(TokenType::OpenBrace,  "expected '{' after 'exit_into' declarator - ");
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
        throw PARSE_ERROR(next, "outcome must be <identifier> (goto) or response list - ");
    }

    auto& edges = ctx.graph->edges;
    ctx.graph->verts[vert].edges = (uint32_t)edges.size();
    while (true) {
        next = ctx.lexer.next();
        switch (next->type) {
        case TokenType::OpenParen: {
            auto& temp = edges.emplace_back();
            temp.condition = ctx.push_next_expr();
            ctx.unwrap_next(TokenType::CloseParen, "ENGINE ERROR UNREACHABLE - ");
            temp.line = ctx.push_str(ctx.unwrap_next(TokenType::StringLiteral, "condition must precede a response line - "));
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
            throw PARSE_ERROR(next, "expected response or end of response list - ");
        };

        auto& resp = edges.back();

        ctx.unwrap_next(TokenType::Arrow, "expected '=>' to denote response outcome - ");
        const auto edge = ctx.unwrap_next((uint32_t)TokenType::Identifier|(uint32_t)TokenType::IntLiteral, "expected <identifier> node id - ");
        resp.edge = ctx.push_vertex(edge);

        next = ctx.lexer.next();
        switch (next->type) {
        case TokenType::Comma:
            resp.modifiers = 0;
            break;

        case TokenType::OpenBrace: {
            parse_modifiers(ctx, resp);
            next = ctx.lexer.next();
            switch (next->type) {
            case TokenType::CloseBrace: return;
            case TokenType::Comma: break;
            default:
                throw PARSE_ERROR(next, "responses in list must be ',' separated - ");
            }
            break; }

        default:
            throw PARSE_ERROR(next, "responses in list must be ',' separated - ");
        }
    }
}


static void parse_modifiers(ParseContext& ctx, _Edge& edge) {
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
            throw PARSE_ERROR(next, "expected flag name or end of modifier list - ");
        }

        {
        bytecode.push_back(Expr::IPushK);
        const auto idx = (uint32_t)bytecode.size();
        bytecode.resize(idx + sizeof(uint32_t));
        memcpy(bytecode.data() + idx, &key, sizeof(uint32_t));
        }

        bytecode.push_back(Expr::IPushC);
        const auto idx = bytecode.size();
        bytecode.resize(idx + sizeof(int64_t));

        next = ctx.lexer.next();
        switch (next->type) {
        case TokenType::PlusEq: {
            const auto val = ctx.unwrap_next(TokenType::IntLiteral, "flag modifier must be integer - ");
            const auto n = std::atoll(val.c_str());
            memcpy(bytecode.data() + idx, &n, sizeof(int64_t));
            bytecode.push_back(Expr::IAddV);
            break; }

        case TokenType::SubEq: {
            const auto val = ctx.unwrap_next(TokenType::IntLiteral, "flag modifier must be integer - ");
            const auto n = -std::atoll(val.c_str());
            memcpy(bytecode.data() + idx, &n, sizeof(int64_t));
            bytecode.push_back(Expr::IAddV);
            break; }

        case TokenType::EqualTo: {
            const auto val = ctx.unwrap_next((uint32_t)TokenType::IntLiteral|(uint32_t)TokenType::Identifier,
                    "flag modifier must be integer or boolean - ");
            uint64_t n = 0;
            if (val == "true") {
                n = 1;
            } else if (val == "false") {
                n = 0;
            } else if (val == "inf") {
                n = UINT32_MAX;
            } else {
                n = (uint64_t)std::atoll(val.c_str());
            }
            memcpy(bytecode.data() + idx, &n, sizeof(uint64_t));
            bytecode.push_back(Expr::ISetV);
            break; }

        default:
            throw PARSE_ERROR(next, "expected '+='|'-='|'=' to specify modifier - ");
        }

        switch (ctx.lexer.peek()->type) {
        case TokenType::CloseBrace: break;
        case TokenType::Comma: ctx.lexer.next(); break;
        default:
            throw PARSE_ERROR(next, "modifiers in list must be ',' separated - ");
        }
    }
}


static void parse_pragma(ParseContext& ctx, std::optional<Token> tok) {
    if (tok->val == "!SET_OR_CREATE") {
        ctx.set_strict = false;
    } else if (tok->val == "!SET_STRICT") {
        ctx.set_strict = true;
    } else if (tok->val == "!LINK_AUDIO") {
        ctx.link_audio = true;
    } else {
        throw PARSE_ERROR(tok, "invalid pragma - ");
    }
}

} } }
