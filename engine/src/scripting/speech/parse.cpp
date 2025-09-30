#include "pch.h"
#include "scripting/speech/graph.h"
#include "scripting/expr.h"
#include "scripting/lexer.h"
#include <stdexcept>


namespace shmy {

namespace detail {

struct ParseContext {
    speech::Graph result;

    Lexer lexer;
    size_t entry_count = 0;

    bool p_set_strict = true;
    bool p_link_audio = false;
};

}

Expr parse_flag_expr(detail::Lexer& lexer);

namespace speech {

using namespace detail;

static bool parse_vertex(ParseContext& ctx);
static Outcome parse_outcome(ParseContext& ctx);
static void set_pragma(ParseContext& ctx, const std::string& pragma);


Graph parse_graph(detail::Lexer&& lexer) {
    auto ctx = detail::ParseContext{ Graph(), std::move(lexer) };
    while (parse_vertex(ctx));
    return ctx.result;
}


static std::string span_to_str(size_t row, size_t col) {
    return std::to_string(row) + ":" + std::to_string(col);
}


static std::string unwrap_token(const std::optional<Token>& value, TokenType expect, const std::string& error) {
    return (value.has_value() && value->type == expect) ? value->val :
        throw std::runtime_error(error + span_to_str(value->row, value->col));
}

static std::string unwrap_token(const std::optional<Token>& value, int expect, const std::string& error) {
    return (value.has_value() && (uint32_t)value->type & expect) ? value->val :
        throw std::runtime_error(error + span_to_str(value->row, value->col));
}

static bool parse_vertex(ParseContext& ctx) {
    auto next = ctx.lexer.next();
    if (!next.has_value()) { return false; }
    while (next->type == TokenType::Pragma) { set_pragma(ctx, next->val); next = ctx.lexer.next(); if (!next.has_value()) { return false; } }

    auto key =  unwrap_token(next, (uint32_t)TokenType::Identifier|(uint32_t)TokenType::IntLiteral, "token unwrapped to wrong type - ");
    auto conditions = Expr::True();
    if (key == "entry") {
        key = key + std::to_string(ctx.entry_count++);
        unwrap_token(ctx.lexer.next(), TokenType::OpenParen, "'entry' vertices must have a condition or 'default' - ");
        conditions = parse_flag_expr(ctx.lexer);
        conditions.strict = ctx.p_set_strict;
    }

                         unwrap_token(ctx.lexer.next(), TokenType::EqualTo,     "expected '=' after vertex declaration - ");
    const auto speaker = unwrap_token(ctx.lexer.next(), TokenType::Identifier,  "vertex definition must start with speaker ID - ");
                         unwrap_token(ctx.lexer.next(), TokenType::Colon,       "expected ':' after speaker ID - ");
                         unwrap_token(ctx.lexer.next(), TokenType::OpenBracket, "expected string array here - ");
    auto lines = std::vector<std::string>();
    while (true) {
        next = ctx.lexer.next();
        switch (next->type) {
        case TokenType::CloseBracket:  goto string_arr_end;
        case TokenType::StringLiteral: lines.push_back(next->val); break;
        default: throw std::runtime_error(std::string("token unwrapped to wrong type - ") + span_to_str(next->row, next->col));
        };
        next = ctx.lexer.next();
        switch (next->type) {
        case TokenType::CloseBracket:  goto string_arr_end;
        case TokenType::Comma:         break;
        default: throw std::runtime_error(std::string("token unwrapped to wrong type - ") + span_to_str(next->row, next->col));
        }
    }
string_arr_end:
    unwrap_token(ctx.lexer.next(), TokenType::Arrow, "token unwrapped to wrong type - ");
    ctx.result[key] = Vertex{ std::move(conditions), std::move(speaker), std::move(lines), parse_outcome(ctx) };
    return true;
}

static Outcome parse_outcome(ParseContext& ctx) {
    auto next = ctx.lexer.next();
    switch (next->type) {
    case TokenType::Identifier:
        if (next->val == "exit") {
            return Goto{ "", true };
        } else if (next->val == "exit_into") {
                                unwrap_token(ctx.lexer.next(), TokenType::OpenBrace,     "token unwrapped to wrong type - ");
            const auto script = unwrap_token(ctx.lexer.next(), TokenType::StringLiteral, "exit_into block must contain a script name - ");
                                unwrap_token(ctx.lexer.next(), TokenType::CloseBrace,    "token unwrapped to wrong type - ");
            return Goto{ script, true };
        } else {
            return Goto{ next->val, false };
        }
    case TokenType::OpenBrace: break;
    default: throw std::runtime_error(std::string("token unwrapped to wrong type - ") + span_to_str(next->row, next->col));
    }

    auto responses = std::vector<Response>();
    while (true) {
        auto conditions = Expr::True();
        auto text = std::string("");
        next = ctx.lexer.next();
        switch (next->type) {
        case TokenType::OpenParen:
            conditions = parse_flag_expr(ctx.lexer);
            conditions.strict = ctx.p_set_strict;
            text = unwrap_token(ctx.lexer.next(), TokenType::StringLiteral, "token unwrapped to wrong type - ");
            break;
        case TokenType::StringLiteral: text = next->val; break;
        case TokenType::CloseBrace: goto after_loop2;
        default: throw std::runtime_error(std::string("token unwrapped to wrong type - ") + span_to_str(next->row, next->col));
        };


                          unwrap_token(ctx.lexer.next(), TokenType::Arrow, "token unwrapped to wrong type - ");
        const auto edge = unwrap_token(ctx.lexer.next(), (uint32_t)TokenType::Identifier|(uint32_t)TokenType::IntLiteral, "token unwrapped to wrong type - ");
        auto modifiers = Expr{};
        next = ctx.lexer.next();
        switch (next->type) {
        case TokenType::Comma: goto this_one;
        case TokenType::OpenBrace: break;
        default: throw std::runtime_error(std::string("token unwrapped to wrong type - ") + span_to_str(next->row, next->col));
        }
        while (true) {
            auto key = std::string("");
            next = ctx.lexer.next();
            switch (next->type) {
            case TokenType::Identifier: key = next->val; break;
            case TokenType::CloseBrace: goto after_loop3;
            default: throw std::runtime_error(std::string("expected modifier or end of list - ") + span_to_str(next->row, next->col));
            }
            unwrap_token(ctx.lexer.next(), TokenType::Colon, "expected colon to denote modification - ");

            {
            const auto n = modifiers.idents.size();
            modifiers.bytecode.push_back(Expr::IPushK);
            const auto idx = modifiers.bytecode.size();
            modifiers.bytecode.resize(idx + sizeof(size_t));
            memcpy(modifiers.bytecode.data() + idx, &n, sizeof(size_t));
            modifiers.idents.push_back(key);
            }

            modifiers.bytecode.push_back(Expr::IPushC);
            const auto idx = modifiers.bytecode.size();
            modifiers.bytecode.resize(idx + sizeof(int64_t));

            next = ctx.lexer.next();
            switch (next->type) {
            case TokenType::Add: {
                const auto val = unwrap_token(ctx.lexer.next(), TokenType::IntLiteral, "flag modifier must be integer - ");
                const auto n = std::atoll(val.c_str());
                memcpy(modifiers.bytecode.data() + idx, &n, sizeof(int64_t));
                modifiers.bytecode.push_back(Expr::IAddV);
                break; }
            case TokenType::Sub: {
                const auto val = unwrap_token(ctx.lexer.next(), TokenType::IntLiteral, "flag modifier must be integer - ");
                const auto n = -std::atoll(val.c_str());
                memcpy(modifiers.bytecode.data() + idx, &n, sizeof(int64_t));
                modifiers.bytecode.push_back(Expr::IAddV);
                break; }
            case TokenType::EqualTo: {
                const auto val = unwrap_token(ctx.lexer.next(), TokenType::IntLiteral, "flag modifier must be integer - ");
                const auto n = std::atoll(val.c_str());
                memcpy(modifiers.bytecode.data() + idx, &n, sizeof(int64_t));
                modifiers.bytecode.push_back(Expr::ISetV);
                break; }
            default: throw std::runtime_error(std::string("expected '+'|'-'|'=' to specify modifier - ") + span_to_str(next->row, next->col));
            }

            next = ctx.lexer.next();
            switch (next->type) {
            case TokenType::CloseBrace: goto after_loop3;
            case TokenType::Comma: break;
            default: throw std::runtime_error(std::string("token unwrapped to wrong type - ") + span_to_str(next->row, next->col));
            }
        }
after_loop3:

        next = ctx.lexer.next();
        switch (next->type) {
        case TokenType::CloseBrace: goto after_loop2;
        case TokenType::Comma: break;
        default: throw std::runtime_error(std::string("token unwrapped to wrong type - ") + span_to_str(next->row, next->col));
        }

this_one:
        modifiers.bytecode.push_back(Expr::IEndOf);
        modifiers.strict = ctx.p_set_strict;

        responses.emplace_back(Response{
            std::move(text),
            std::move(edge),
            std::move(conditions),
            std::move(modifiers),
        });
    }
after_loop2:

    return responses;
}


static void set_pragma(ParseContext& ctx, const std::string& pragma) {
    if (pragma == "!SET_OR_CREATE") {
        ctx.p_set_strict = false;
    } else if (pragma == "!SET_STRICT") {
        ctx.p_set_strict = true;
    } else if (pragma == "!LINK_AUDIO") {
        ctx.p_link_audio = true;
    }
}

} }
