#include "pch.h"
#include "speech_graph.h"
#include "flag_expr.h"
#include "lexer.h"
#include <stdexcept>


using SpeechVertexTuple = std::pair<std::string, SpeechVertex>;

std::optional<SpeechVertexTuple> parse_vertex(Lexer& lexer, size_t& entrycount);
SpeechOutcome parse_outcome(Lexer& lexer);
FlagExpr parse_flag_expr(Lexer& lexer);
void set_pragma(const std::string& pragma);


static bool p_set_strict = true;
static size_t p_pool_size = 16;

SpeechGraph parse_speechgraph(Lexer lexer) {
    auto result = SpeechGraph();
    size_t entrycount = 0;
    p_set_strict = true;

    while (auto pair = parse_vertex(lexer, entrycount)) {
        // const auto [key, val] = *pair;
        // result[pair->first] = std::move(pair->second);
        result.emplace(pair->first, std::move(pair->second));
    }

    return result;
}


std::string span_to_str(size_t row, size_t col) {
    return std::to_string(row) + ":" + std::to_string(col);
}


static std::string unwrap_token(const std::optional<Token>& value, TokenType expect) {
    return (value.has_value() && value->type == expect) ? value->val :
        throw std::invalid_argument(std::string("token unwrapped to wrong type - ") + span_to_str(value->row, value->col));
}

static std::string unwrap_token(const std::optional<Token>& value, int expect) {
    return (value.has_value() && (uint32_t)value->type & expect) ? value->val :
        throw std::invalid_argument(std::string("token unwrapped to wrong type - ") + span_to_str(value->row, value->col));
}

std::optional<SpeechVertexTuple> parse_vertex(Lexer& lexer, size_t& entrycount) {
    auto next = lexer.next();
    if (!next.has_value()) { return {}; }
    while (next->type == TokenType::Pragma) { set_pragma(next->val); next = lexer.next(); if (!next.has_value()) { return {}; } }
    auto key =  unwrap_token(next, (uint32_t)TokenType::Identifier|(uint32_t)TokenType::IntLiteral);
    auto conditions = FlagExpr::True();
    if (key == "entry") {
        key = key + std::to_string(entrycount++);
        unwrap_token(lexer.next(), TokenType::OpenParen);
        conditions = parse_flag_expr(lexer);
    }

                         unwrap_token(lexer.next(), TokenType::EqualTo);
    const auto speaker = unwrap_token(lexer.next(), TokenType::Identifier);
                         unwrap_token(lexer.next(), TokenType::Colon);
                         unwrap_token(lexer.next(), TokenType::OpenBracket);
    auto lines = std::vector<std::string>();
    while (true) {
        next = lexer.next();
        switch (next->type) {
        case TokenType::StringLiteral: lines.push_back(next->val); break;
        case TokenType::CloseBracket:  goto after_loop1;
        default: throw std::invalid_argument(std::string("token unwrapped to wrong type - ") + span_to_str(next->row, next->col));
        };
        next = lexer.next();
        switch (next->type) {
        case TokenType::CloseBracket: goto after_loop1;
        case TokenType::Comma: break;
        default: throw std::invalid_argument(std::string("token unwrapped to wrong type - ") + span_to_str(next->row, next->col));
        }
    }
after_loop1:
    unwrap_token(lexer.next(), TokenType::Arrow);
    return { { key, SpeechVertex{ std::move(conditions), speaker, lines, parse_outcome(lexer) } } };
}

SpeechOutcome parse_outcome(Lexer& lexer) {
    auto next = lexer.next();
    switch (next->type) {
    case TokenType::Identifier:
        if (next->val== "exit") {
            return SpeechExit{};
        } else if (next->val == "exit_into") {
            unwrap_token(lexer.next(), TokenType::OpenBrace);
            const auto script = unwrap_token(lexer.next(), TokenType::StringLiteral);
            unwrap_token(lexer.next(), TokenType::CloseBrace);
            return SpeechExitInto(script);
        } else {
            return SpeechGoto(next->val);
        }
    case TokenType::OpenBrace: break;
    default: throw std::invalid_argument(std::string("token unwrapped to wrong type - ") + span_to_str(next->row, next->col));
    }

    auto responses = std::vector<SpeechResponse>();
    while (true) {
        auto conditions = FlagExpr::True();
        auto text = std::string("");
        next = lexer.next();
        switch (next->type) {
        case TokenType::Question:
                   unwrap_token(lexer.next(), TokenType::OpenParen);
            conditions = parse_flag_expr(lexer);
            text = unwrap_token(lexer.next(), TokenType::StringLiteral);
            break;
        case TokenType::StringLiteral: text = next->val; break;
        case TokenType::CloseBrace: goto after_loop2;
        default: throw std::invalid_argument(std::string("token unwrapped to wrong type - ") + span_to_str(next->row, next->col));
        };

                          unwrap_token(lexer.next(), TokenType::Arrow);
        const auto edge = unwrap_token(lexer.next(), (uint32_t)TokenType::Identifier|(uint32_t)TokenType::IntLiteral);
        auto flags = std::unordered_map<std::string, FlagModifier>();
        next = lexer.next();
        switch (next->type) {
        case TokenType::Comma: goto this_one;
        case TokenType::OpenBrace: break;
        default: throw std::invalid_argument(std::string("token unwrapped to wrong type - ") + span_to_str(next->row, next->col));
        }
        while (true) {
            auto key = std::string("");
            next = lexer.next();
            switch (next->type) {
            case TokenType::Identifier: key = next->val; break;
            case TokenType::CloseBrace: goto after_loop3;
            default: throw std::invalid_argument(std::string("token unwrapped to wrong type - ") + span_to_str(next->row, next->col));
            }
                             unwrap_token(lexer.next(), TokenType::Colon);
            const auto flg = unwrap_token(lexer.next(), TokenType::Identifier);
                             unwrap_token(lexer.next(), TokenType::OpenParen);
            const auto val = unwrap_token(lexer.next(), TokenType::IntLiteral);
                             unwrap_token(lexer.next(), TokenType::CloseParen);
            auto modifier = FlagModifier();
            if (flg == "Set") {
                modifier = FlagSet{ std::atoi(val.c_str()), p_set_strict };
            } else if (flg == "Add") {
                modifier = FlagAdd{ std::atoi(val.c_str()), p_set_strict };
            } else if (flg == "Sub") {
                modifier = FlagSub{ std::atoi(val.c_str()), p_set_strict };
            } else {
                throw std::invalid_argument(std::string("token unwrapped to wrong type - ") + span_to_str(next->row, next->col));
            };
            flags[key] = modifier;
            next = lexer.next();
            switch (next->type) {
            case TokenType::CloseBrace: goto after_loop3;
            case TokenType::Comma: break;
            default: throw std::invalid_argument(std::string("token unwrapped to wrong type - ") + span_to_str(next->row, next->col));
            }
        }
after_loop3:

        next = lexer.next();
        switch (next->type) {
        case TokenType::CloseBrace: goto after_loop2;
        case TokenType::Comma: break;
        default: throw std::invalid_argument(std::string("token unwrapped to wrong type - ") + span_to_str(next->row, next->col));
        }

this_one:
        responses.emplace_back(SpeechResponse{
            std::move(conditions),
            text,
            edge,
            flags
        });
    }
after_loop2:

    return responses;
}


void set_pragma(const std::string& pragma) {
    if (pragma == "!SET_OR_CREATE") {
        p_set_strict = false;
    } else if (pragma == "!SET_STRICT") {
        p_set_strict = true;
    } else if (pragma == "!POOL_SIZE_HUGE") {
        p_pool_size = 64;
    } else if (pragma == "!POOL_SIZE_DOUBLE") {
        p_pool_size = 32;
    } else if (pragma == "!POOL_SIZE_SINGLE") {
        p_pool_size = 16;
    }
}

