#pragma once
#include <string_view>


namespace shmy::detail {


struct Token {
    struct Span {
        size_t row = 0;
        size_t col = 0;
    };
    enum class Type {
        MyFault = -2,
        Error = -1,
        Ident,
        IntLit,
        StrLit,
        LParen,
        RParen,
        Lnot,
        Lor,
        Land,
        Add,
        Sub,
        Mul,
        Div,
        Equ,
        Neq,
        Gth,
        Lth,
        Leq,
        Geq,
        Colon,
        Eof,
    } type;
    std::string_view value;
    Span span;
};

class Lexer {
private:
    enum class State {
        Void,
        Symbol,
        Ident,
        Number,
        String,
        StringEsc,
    };

    const char* base;
    const char* ptr;
    Token::Span pos = { 1, 0 };
    bool wasnull = false;
    State state = State::Void;
    Token cache;

    Token step();

public:
    Lexer(const char* src);

    Token peek();
    Token next();
};

}
