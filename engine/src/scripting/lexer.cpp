#include "pch.h"
#include "lexer.h"


Lexer::Lexer(const std::string& _src) : src(&_src) {}

std::optional<Token> Lexer::next() {
    auto token = std::string("");

    if (state == State::Void && last.has_value()) {
        const auto c = *last;
        last = {};
        switch (c) {
        case '#': state = State::Comment; break;
        case '>': state = State::Comparator; last = c; break;
        case '<': state = State::Comparator; last = c; break;
        case '!': state = State::Comparator; last = c; break;
        case '(': return Token{ TokenType::OpenParen,    "(" };
        case ')': return Token{ TokenType::CloseParen,   ")" };
        case '{': return Token{ TokenType::OpenBrace,    "{" };
        case '}': return Token{ TokenType::CloseBrace,   "}" };
        case '[': return Token{ TokenType::OpenBracket,  "[" };
        case ']': return Token{ TokenType::CloseBracket, "]" };
        case ',': return Token{ TokenType::Comma,        "," };
        case ':': return Token{ TokenType::Colon,        ":" };
        case '?': return Token{ TokenType::Question,     "?" };
        default:
            if (('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z')) {
                state = State::Identifier;
                token.push_back(c);
            } else if ('0' <= c && c <= '9') {
                state = State::Number;
                token.push_back(c);
            } else if (c == ' ' || c == '\t' || c == '\n' || c == '\r') {
                break;
            } else {
                throw LexerError(std::string("invalid character '") + c + "' in script body");
            }
        }
    }

    while (index < src->size()) {
        const auto c = (*src)[index++];

        switch (state) {
        case State::Void:
            if (('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z')) {
                state = State::Identifier;
                token.push_back(c);
            } else if ('0' <= c && c <= '9') {
                state = State::Number;
                token.push_back(c);
            } else if (c == ' ' || c == '\t' || c == '\n' || c == '\r') {
            } else {
                switch (c) {
                case '=': state = State::Equals; break;
                case '"': state = State::String;  break;
                case '#': state = State::Comment; break;
                case '>': state = State::Comparator; last = c; break;
                case '<': state = State::Comparator; last = c; break;
                case '!': state = State::Comparator; last = c; break;
                case '(': return Token{ TokenType::OpenParen,    "(" };
                case ')': return Token{ TokenType::CloseParen,   ")" };
                case '{': return Token{ TokenType::OpenBrace,    "{" };
                case '}': return Token{ TokenType::CloseBrace,   "}" };
                case '[': return Token{ TokenType::OpenBracket,  "[" };
                case ']': return Token{ TokenType::CloseBracket, "]" };
                case ',': return Token{ TokenType::Comma,        "," };
                case ':': return Token{ TokenType::Colon,        ":" };
                case '?': return Token{ TokenType::Question,     "?" };
                case '\0': return {};
                default: throw LexerError(std::string("invalid character '") + c + "' in script body");
                }
            }
            break;
        case State::Identifier:
            if (('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z') || ('0' <= c && c <= '9') || c == '_') {
                token.push_back(c);
            } else if (c == '(' || c == ')' || c == '{' || c == '}' || c == '[' || c == ']' || c == ',' ||
                       c == ':' || c == '?' || c == '>' || c == '<' || c == '!') {
                state = State::Void;
                last = c;
                return Token{ TokenType::Identifier, token };
            } else if (c == '#') {
                state = State::Comment;
                last = c;
                return Token{ TokenType::Identifier, token };
            } else if (c == '=') {
                state = State::Equals;
                return Token{ TokenType::Identifier, token };
            } else if (c == '"') {
                state = State::String;
                return Token{ TokenType::Identifier, token };
            } else if (c == ' ' || c == '\t' || c == '\n' || c == '\r') {
                state = State::Void;
                return Token{ TokenType::Identifier, token };
            } else {
                throw LexerError(std::string("invalid character '") + c + "' after identifier");
            }
            break;
        case State::Number:
            if ('0' <= c && c <= '9') {
                token.push_back(c);
            } else if (c == '.') {
                token.push_back(c);
                state = State::NumberDecimal;
            } else if (c == '(' || c == ')' || c == '{' || c == '}' || c == '[' || c == ']' || c == ',' ||
                       c == ':' || c == '?' || c == '>' || c == '<' || c == '!') {
                state = State::Void;
                last = c;
                return Token{ TokenType::IntLiteral, token };
            } else if (c == '#') {
                state = State::Comment;
                last = c;
                return Token{ TokenType::IntLiteral, token };
            } else if (c == '=') {
                state = State::Equals;
                return Token{ TokenType::IntLiteral, token };
            } else if (c == '"') {
                state = State::String;
                return Token{ TokenType::IntLiteral, token };
            } else if (c == ' ' || c == '\t' || c == '\n' || c == '\r') {
                state = State::Void;
                return Token{ TokenType::IntLiteral, token };
            } else {
                throw LexerError(std::string("invalid character '") + c + "' after int literal");
            }
            break;
        case State::NumberDecimal:
            if ('0' <= c && c <= '9') {
                token.push_back(c);
                state = State::NumberDecimalNumber;
            } else {
                throw LexerError("invalid trailing period after int literal");
            }
            break;
        case State::NumberDecimalNumber:
            if ('0' <= c && c <= '9') {
                token.push_back(c);
            } else if (c == '(' || c == ')' || c == '{' || c == '}' || c == '[' || c == ']' || c == ',' ||
                       c == ':' || c == '?' || c == '>' || c == '<' || c == '!') {
                state = State::Void;
                last = c;
                return Token{ TokenType::FloatLiteral, token };
            } else if (c == '#') {
                state = State::Comment;
                last = c;
                return Token{ TokenType::FloatLiteral, token };
            } else if (c == '=') {
                state = State::Equals;
                return Token{ TokenType::FloatLiteral, token };
            } else if (c == '"') {
                state = State::String;
                return Token{ TokenType::FloatLiteral, token };
            } else if (c == ' ' || c == '\t' || c == '\n' || c == '\r') {
                state = State::Void;
                return Token{ TokenType::FloatLiteral, token };
            } else {
                throw LexerError(std::string("invalid character '") + c + "' after float literal");
            }
            break;
        case State::String:
            switch (c) {
            case '"':
                state = State::Void;
                return Token{ TokenType::StringLiteral, token };
            case '\\':
                state = State::StringEscape;
                break;
            default:
                token.push_back(c);
                break;
            }
            break;
        case State::StringEscape:
            state = State::String;
            switch (c) {
            case '"':
                token.push_back('"');
                break;
            case '\\':
                token.push_back('\\');
                break;
            case 't':
                token.push_back('\t');
                break;
            case 'n':
                token.push_back('\n');
                break;
            default:
                throw LexerError(std::string("invalid character '") + c + "' in escape sequence");
            }
            break;
        case State::Equals:
            state = State::Void;
            switch (c) {
            case '>': return Token{ TokenType::Arrow, "=>" };
            case '=': return Token{ TokenType::EqualEq, "==" };
            default:
                last = c;
                return Token{ TokenType::EqualTo, "=" };
            }
            break;
        case State::Comparator:
            state = State::Void;
            if (c == '=') {
                const auto temp = *last;
                last = {};
                switch (temp) {
                case '>': return Token{ TokenType::GreaterEq, ">=" };
                case '<': return Token{ TokenType::LessEq, "<=" };
                case '!': return Token{ TokenType::BangEq, "!=" };
                default: throw LexerError("state should be unreachable");
                }
            } else {
                const auto temp = *last;
                last = c;
                switch (temp) {
                case '>': return Token{ TokenType::GreaterThan, ">" };
                case '<': return Token{ TokenType::LessThan, "<" };
                case '!': return Token{ TokenType::Bang, "!" };
                default: throw LexerError("programming error: state should be unreachable");
                }
            }
            break;
        case State::Comment:
            if (c == '\n') {
                state = State::Void;
            }
            break;
        default:
            throw LexerError("programming error: state should be unreachable");
        }
    }

    if (!token.empty()) {
        switch (state) {
        case State::Identifier:          return Token{ TokenType::Identifier,   token };
        case State::Number:              return Token{ TokenType::IntLiteral,   token };
        case State::NumberDecimalNumber: return Token{ TokenType::FloatLiteral, token };
        default: throw LexerError("invalid trailing characters in final token - should be unreachable");
        }
    }

    return {};
}

