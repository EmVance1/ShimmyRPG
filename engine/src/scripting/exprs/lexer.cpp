#include "pch.h"
#include "scripting/exprs/lexer.h"
#include <string.h>


namespace shmy::detail {

static bool is_num(char c) {
    return '0' <= c && c <= '9';
}
static bool is_ident_begin(char c) {
    return ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z') || c == '_';
}
static bool is_ident(char c) {
    return is_ident_begin(c) || is_num(c) || c == '.';
}
static bool is_space(char c, bool* wasnull) {
    if (c == '\0') *wasnull = true;
    return c == ' ' || c == '\n' || c == '\r' || c == '\t' || c == '\0';
}


Lexer::Lexer(const char* text) :
    base(text),
    ptr(text),
    cache(step())
{}

Token Lexer::peek() {
    return cache;
}

Token Lexer::next() {
    const Token temp = cache;
    cache = step();
    return temp;
}


Token Lexer::step() {
    const char* tbeg = NULL;
    size_t tlen = 0;
    Token::Span tspan = pos;

    while (!wasnull) {
        Token::Span p = pos;
        const char c = *(ptr++);
        if (c == '\n') { p.row++; p.col = 0; } else { p.col++; }

        switch (state) {
        case Lexer::State::Void:
            if (is_space(c, &wasnull)) {
                // continue
            } else if (is_ident_begin(c)) {
                state = Lexer::State::Ident;
                tbeg = ptr-1;
                tlen = 1;
                tspan = p;
            } else if (is_num(c)) {
                state = Lexer::State::Number;
                tbeg = ptr-1;
                tlen = 1;
                tspan = p;
            } else {
                tbeg = ptr-1;
                tlen = 1;
                tspan = p;
                pos = p;
                switch (c) {
                case '(': return Token{ .type=Token::Type::LParen, .value={ tbeg, 1 }, .span=tspan };
                case ')': return Token{ .type=Token::Type::LParen, .value={ tbeg, 1 }, .span=tspan };
                case '<': case '>': case '=': case '!': case '&': case '|':
                    state = Lexer::State::Symbol; break;
                case '"':
                    state = Lexer::State::String; break;
                default:
                    return Token{ .type=Token::Type::Error, .value="invalid token", .span=tspan };
                }
            }
            break;

        case Lexer::State::Ident:
            if (is_ident(c)) {
                tlen++;
            } else {
                if (is_space(c, &wasnull)) {
                    pos = p;
                } else {
                    ptr--;
                }
                state = Lexer::State::Void;
                return Token{ .type=Token::Type::Ident, .value={ tbeg, tlen }, .span=tspan };
            }
            break;

        case Lexer::State::Number:
            if (is_num(c)) {
                tlen++;
            } else if (is_space(c, &wasnull)) {
                pos = p;
                state = Lexer::State::Void;
                return Token{ .type=Token::Type::IntLit, .value={ tbeg, tlen }, .span=tspan };
            } else {
                ptr--;
                state = Lexer::State::Void;
                return Token{ .type=Token::Type::IntLit, .value={ tbeg, tlen }, .span=tspan };
            }
            break;

        case Lexer::State::String:
            tlen++;
            if (c == '"') {
                pos = p;
                state = Lexer::State::Void;
                return Token{ .type=Token::Type::StrLit, .value={ tbeg+1, tlen-2 }, .span=tspan };
            } else if (c == '\\') {
                state = Lexer::State::StringEsc;
            }
            break;

        case Lexer::State::StringEsc:
            tlen++;
            state = Lexer::State::String;
            break;

        case Lexer::State::Symbol:
            state = Lexer::State::Void;
            if (c == '=') {
                pos = p;
                switch (*tbeg) {
                case '<': return Token{ .type=Token::Type::Leq,   .value={ tbeg, 2 }, .span=tspan };
                case '>': return Token{ .type=Token::Type::Geq,   .value={ tbeg, 2 }, .span=tspan };
                case '=': return Token{ .type=Token::Type::Equ,   .value={ tbeg, 2 }, .span=tspan };
                case '!': return Token{ .type=Token::Type::Neq,   .value={ tbeg, 2 }, .span=tspan };
                default:
                    return Token{ .type=Token::Type::Error, .value="invalid token", .span=tspan };
                }
            } else if (c == '&' && *tbeg == '&') {
                pos = p;
                return Token{ .type=Token::Type::Land, .value={ tbeg, 2 }, .span=tspan };
            } else if (c == '|' && *tbeg == '|') {
                pos = p;
                return Token{ .type=Token::Type::Lor,  .value={ tbeg, 2 }, .span=tspan };
            } else {
                ptr--;
                switch (*tbeg) {
                case '<': return Token{ .type=Token::Type::Lth,   .value={ tbeg, 1 }, .span=tspan };
                case '>': return Token{ .type=Token::Type::Gth,   .value={ tbeg, 1 }, .span=tspan };
                case '!': return Token{ .type=Token::Type::Lnot,  .value={ tbeg, 1 }, .span=tspan };
                case '&': case '|':
                    return Token{ .type=Token::Type::Error,   .value="invalid token", .span=tspan };
                default:
                    return Token{ .type=Token::Type::MyFault, .value="CODING ERROR: switch should never default", .span=tspan };
                }
            }
            break;
        }
        pos = p;
    }

    return Token{ .type=Token::Type::Eof, .value={ "[[EOF]]", 0 }, .span=pos };
}

}
