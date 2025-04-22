#include "pch.h"
#include "speech_graph.h"
#include "flag_expr.h"
#include "lexer.h"


using SpeechVertexTuple = std::pair<std::string, SpeechVertex>;


std::optional<SpeechVertexTuple> parse_vertex(Lexer& lexer, size_t& entrycount);
SpeechOutcome parse_outcome(Lexer& lexer);
FlagExpr parse_flag_expr(Lexer& lexer);


SpeechGraph parse_speechgraph(Lexer lexer) {
    auto result = SpeechGraph();
    size_t entrycount = 0;

    while (auto pair = parse_vertex(lexer, entrycount)) {
        const auto [key, val] = *pair;
        result[key] = val;
    }

    return result;
}


static std::string unwrap_token(const std::optional<Token>& value, TokenType expect) {
    return (value.has_value() && value->type == expect) ? value->value : throw std::exception("WRONG TYPE");
}

static std::string unwrap_token(const std::optional<Token>& value, int expect) {
    return (value.has_value() && (int)value->type & expect) ? value->value : throw std::exception("WRONG TYPE");
}

std::optional<SpeechVertexTuple> parse_vertex(Lexer& lexer, size_t& entrycount) {
    auto next = lexer.next();
    if (!next.has_value()) { return {}; }
    auto key =  unwrap_token(next, (int)TokenType::Identifier|(int)TokenType::IntLiteral);
    auto conditions = FlagExpr::Value(1);
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
        case TokenType::StringLiteral: lines.push_back(next->value); break;
        case TokenType::CloseBracket:  goto after_loop1;
        default: throw std::exception();
        };
        next = lexer.next();
        switch (next->type) {
        case TokenType::CloseBracket: goto after_loop1;
        case TokenType::Comma: break;
        default: throw std::exception();
        }
    }
after_loop1:
    unwrap_token(lexer.next(), TokenType::Arrow);
    const auto outcome = parse_outcome(lexer);
    return { { key, SpeechVertex{ conditions, speaker, lines, outcome } } };
}

SpeechOutcome parse_outcome(Lexer& lexer) {
    auto next = lexer.next();
    switch (next->type) {
    case TokenType::Identifier:
        if (next->value == "exit") {
            return SpeechExit{};
        } else {
            return SpeechGoto(next->value);
        }
    case TokenType::OpenBrace: break;
    default: throw std::exception();
    }

    auto responses = std::vector<SpeechResponse>();
    while (true) {
        auto conditions = FlagExpr::Value(1);
        auto text = std::string("");
        next = lexer.next();
        switch (next->type) {
        case TokenType::Question:
                   unwrap_token(lexer.next(), TokenType::OpenParen);
            conditions = parse_flag_expr(lexer);
            text = unwrap_token(lexer.next(), TokenType::StringLiteral);
            break;
        case TokenType::StringLiteral: text = next->value; break;
        case TokenType::CloseBrace: goto after_loop2;
        default: std::exception();
        };

                          unwrap_token(lexer.next(), TokenType::Arrow);
        const auto edge = unwrap_token(lexer.next(), (int)TokenType::Identifier|(int)TokenType::IntLiteral);
        auto flags = std::unordered_map<std::string, FlagModifier>();
        next = lexer.next();
        switch (next->type) {
        case TokenType::Comma: goto this_one;
        case TokenType::OpenBrace: break;
        default: throw std::exception();
        }
        while (true) {
            auto key = std::string("");
            next = lexer.next();
            switch (next->type) {
            case TokenType::Identifier: key = next->value; break;
            case TokenType::CloseBrace: goto after_loop3;
            default: throw std::exception();
            }
                             unwrap_token(lexer.next(), TokenType::Colon);
            const auto flg = unwrap_token(lexer.next(), TokenType::Identifier);
                             unwrap_token(lexer.next(), TokenType::OpenParen);
            const auto val = unwrap_token(lexer.next(), TokenType::IntLiteral);
                             unwrap_token(lexer.next(), TokenType::CloseParen);
            auto modifier = FlagModifier();
            if (flg == "Set") {
                modifier = FlagSet{ std::atoi(val.c_str()) };
            } else if (flg == "Add") {
                modifier = FlagAdd{ std::atoi(val.c_str()) };
            } else if (flg == "Sub") {
                modifier = FlagSub{ std::atoi(val.c_str()) };
            } else {
                throw std::exception();
            };
            flags[key] = modifier;
            next = lexer.next();
            switch (next->type) {
            case TokenType::CloseBrace: goto after_loop3;
            case TokenType::Comma: break;
            default: throw std::exception();
            }
        }
after_loop3:

        next = lexer.next();
        switch (next->type) {
        case TokenType::CloseBrace: goto after_loop2;
        case TokenType::Comma: break;
        default: throw std::exception();
        }

this_one:
        responses.push_back(SpeechResponse{
            std::move(conditions),
            text,
            edge,
            flags,
        });
    }
after_loop2:

    return responses;
}

