#pragma once
#include <string>
#include <optional>


class LexerError : public std::exception {
private:
    std::string m_str;
public:
    LexerError(const std::string& str) : m_str(str) {}

    const char* what() const override { return m_str.c_str(); }
};


enum class TokenType {
    Identifier    = 1 << 0,
    FloatLiteral  = 1 << 1,
    IntLiteral    = 1 << 2,
    StringLiteral = 1 << 3,
    OpenParen     = 1 << 4,
    CloseParen    = 1 << 5,
    OpenBrace     = 1 << 6,
    CloseBrace    = 1 << 7,
    OpenBracket   = 1 << 8,
    CloseBracket  = 1 << 9,
    Comma         = 1 << 10,
    Colon         = 1 << 11,
    Arrow         = 1 << 12,
    Question      = 1 << 13,
    Bang          = 1 << 14,
    LogOr         = 1 << 15,
    LogAnd        = 1 << 16,
    GreaterThan   = 1 << 17,
    LessThan      = 1 << 18,
    EqualEq       = 1 << 19,
    BangEq        = 1 << 20,
    LessEq        = 1 << 21,
    GreaterEq     = 1 << 22,
    EqualTo       = 1 << 23,
};

struct Token {
    enum TokenType type;
    std::string value;
};


class Lexer {
private:
    enum class State {
        Void,
        Identifier,
        Number,
        NumberDecimal,
        NumberDecimalNumber,
        String,
        StringEscape,
        Equals,
        Comparator,
        Comment,
    };

    const std::string* src;
    State state = State::Void;
    size_t index = 0;
    std::optional<char> last = {};

public:
    Lexer(const std::string& src);

    std::optional<Token> next();
};

