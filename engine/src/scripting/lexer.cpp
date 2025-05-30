#include "pch.h"
#include "lexer.h"


constexpr static bool SYM_LUT[] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 1, 1, 1, 0, 0, 1, 0, 1, 1, 1, 1, 1, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 1, 1, 1,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
};

constexpr static bool WS_LUT[] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
};


Lexer::Lexer(const std::string& _src) : src(_src.c_str()) {}

std::optional<Token> Lexer::next() {
    char token[512] = { 0 };
    size_t tok_len = 0;

    while (const char c = src[index++]) {
        if (c == '\n') { row++; col = 0; } else { col++; }

        switch (state) {
        case State::Void:
            if (('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z') || (c == '_')) {
                state = State::Identifier;
                token[tok_len++] = c;
            } else if ('0' <= c && c <= '9') {
                state = State::Number;
                token[tok_len++] = c;
            } else if (!WS_LUT[(int)c]) {
                switch (c) {
                case '"': state = State::String;  break;
                case '#': state = State::Comment; break;
                case '=': state = State::Compound; token[tok_len++] = c; break;
                case '>': state = State::Compound; token[tok_len++] = c; break;
                case '<': state = State::Compound; token[tok_len++] = c; break;
                case '!': state = State::Compound; token[tok_len++] = c; break;
                case '(': return Token{ TokenType::OpenParen,    "(", row, col };
                case ')': return Token{ TokenType::CloseParen,   ")", row, col };
                case '{': return Token{ TokenType::OpenBrace,    "{", row, col };
                case '}': return Token{ TokenType::CloseBrace,   "}", row, col };
                case '[': return Token{ TokenType::OpenBracket,  "[", row, col };
                case ']': return Token{ TokenType::CloseBracket, "]", row, col };
                case ',': return Token{ TokenType::Comma,        ",", row, col };
                case ':': return Token{ TokenType::Colon,        ":", row, col };
                case '?': return Token{ TokenType::Question,     "?", row, col };
                case '&': return Token{ TokenType::LogAnd,       "&", row, col };
                case '|': return Token{ TokenType::LogOr,        "|", row, col };
                case '\0': return {};
                default: throw LexerError(std::string("invalid character '") + c + "' in script body - "
                             + std::to_string(row) + ":" + std::to_string(col));
                }
            }
            break;
        case State::Identifier:
            if (('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z') || ('0' <= c && c <= '9') || c == '_') {
                token[tok_len++] = c;
            } else if (SYM_LUT[(int)c]) {
                state = State::Void;
                index--;
                return Token{ TokenType::Identifier, token, row, col };
            } else if (WS_LUT[(int)c]) {
                state = State::Void;
                return Token{ TokenType::Identifier, token, row, col };
            } else {
                throw LexerError(std::string("invalid character '") + c + "' after identifier - "
                             + std::to_string(row) + ":" + std::to_string(col));
            }
            break;
        case State::Number:
            if ('0' <= c && c <= '9') {
                token[tok_len++] = c;
            } else if (SYM_LUT[(int)c]) {
                state = State::Void;
                index--;
                return Token{ TokenType::IntLiteral, token, row, col };
            } else if (WS_LUT[(int)c]) {
                state = State::Void;
                return Token{ TokenType::IntLiteral, token, row, col };
            } else {
                throw LexerError(std::string("invalid character '") + c + "' after int literal - "
                             + std::to_string(row) + ":" + std::to_string(col));
            }
            break;
        case State::String:
            switch (c) {
            case '"':
                state = State::Void;
                return Token{ TokenType::StringLiteral, token, row, col };
            case '\\':
                state = State::StringEscape;
                break;
            default:
                token[tok_len++] = c;
                break;
            }
            break;
        case State::StringEscape:
            state = State::String;
            switch (c) {
            case '"':  token[tok_len++] = '"';  break;
            case '\\': token[tok_len++] = '\\'; break;
            case 't':  token[tok_len++] = '\t'; break;
            case 'n':  token[tok_len++] = '\n'; break;
            default: throw LexerError(std::string("invalid character '") + c + "' in escape sequence - "
                             + std::to_string(row) + ":" + std::to_string(col));
            }
            break;
        case State::Compound:
            state = State::Void;
            if (c == '=') {
                switch (token[0]) {
                case '>': return Token{ TokenType::GreaterEq, ">=", row, col };
                case '<': return Token{ TokenType::LessEq,    "<=", row, col };
                case '!': return Token{ TokenType::BangEq,    "!=", row, col };
                case '=': return Token{ TokenType::EqualEq,   "==", row, col };
                default: throw LexerError(std::string("programming error: state '") + token[0] + "' should be unreachable - cmp1");
                }
            } else if (token[0] == '=' && c == '>') {
                return Token{ TokenType::Arrow, "=>", row, col };
            } else {
                index--;
                switch (token[0]) {
                case '>': return Token{ TokenType::GreaterThan, ">", row, col };
                case '<': return Token{ TokenType::LessThan,    "<", row, col };
                case '!': return Token{ TokenType::Bang,        "!", row, col };
                case '=': return Token{ TokenType::EqualTo,     "=", row, col };
                default: throw LexerError(std::string("programming error: state '") + token[0] + "' should be unreachable - cmp2");
                }
            }
            break;
        case State::Comment:
            if (c == '\n') {
                state = State::Void;
                if (tok_len > 0 && token[0] == '!') {
                    return Token{ TokenType::Pragma, token, row, col };
                } else {
                    memset(token, 0, 512);
                    tok_len = 0;
                }
            } else {
                token[tok_len++] = c;
            }
            break;
        }
    }

    if (tok_len != 0) {
        switch (state) {
        case State::Identifier:          return Token{ TokenType::Identifier,   token, row, col };
        case State::Number:              return Token{ TokenType::IntLiteral,   token, row, col };
        default: throw LexerError("invalid trailing characters in final token - should be unreachable");
        }
    }

    return {};
}

