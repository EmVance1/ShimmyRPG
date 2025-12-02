#pragma once
#include <string>
#include <optional>


namespace shmy { namespace speech { namespace detail {

class LexerError : public std::exception {
private:
    std::string m_str;
public:
    LexerError(const std::string& str) : m_str(str) {}

    const char* what() const noexcept override { return m_str.c_str(); }
};


enum class TokenType : uint32_t {
    Identifier    = 1,
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
    Add           = 1 << 24,
    Sub           = 1 << 25,
    Pragma        = 1 << 26,
};

struct Token {
    std::string val;
    TokenType type;
    size_t row = 0;
    size_t col = 0;
};

class Lexer {
private:
    enum class State : uint8_t {
        Void,
        Identifier,
        Number,
        String,
        StringEscape,
        Compound,
        Comment,
    };

    const char* src;
    State state = State::Void;
    size_t index = 0;
    size_t row = 1;
    size_t col = 0;
    std::optional<Token> peeked;

    std::optional<Token> lex();

public:
    Lexer(const std::string& src);

    std::optional<Token> peek();
    std::optional<Token> next();
};

} } }
