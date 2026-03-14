#include "pch.h"
#include "scripting/expr.h"
#include "scripting/exprs/lexer.h"


namespace shmy::detail {

struct ParseContext {
    Lexer lexer;
    std::unordered_map<std::string_view, uint32_t> s_LUT;
};


struct Assoc { float lhs; float rhs; };

static Assoc binding(Token t) {
    switch (t.type) {
    case Token::Type::Lor:
        return Assoc{ 1.1f, 1.f };
    case Token::Type::Land:
        return Assoc{ 2.1f, 2.f };
    case Token::Type::Equ: case Token::Type::Neq:
        return Assoc{ 3.1f, 3.f };
    case Token::Type::Lth: case Token::Type::Gth: case Token::Type::Leq: case Token::Type::Geq:
        return Assoc{ 4.1f, 4.f };
    case Token::Type::Add: case Token::Type::Sub:
        return Assoc{ 5.1f, 5.f };
    case Token::Type::Mul: case Token::Type::Div:
        return Assoc{ 6.1f, 6.f };
    case Token::Type::Colon:
        return Assoc{ 7.1f, 7.f };
    case Token::Type::Lnot:
        return Assoc{ 0.f, 8.f };

    default:
        return Assoc{ -INFINITY, -INFINITY };
    }
}
static Expr::Instr opcode(Token t) {
    switch (t.type) {
    case Token::Type::Lor:   return Expr::ILogOr;
    case Token::Type::Land:  return Expr::ILogAnd;
    case Token::Type::Lnot:  return Expr::ILogNot;
    case Token::Type::Equ:   return Expr::ICmpEq;
    case Token::Type::Neq:   return Expr::ICmpNe;
    case Token::Type::Lth:   return Expr::ICmpLt;
    case Token::Type::Gth:   return Expr::ICmpGt;
    case Token::Type::Leq:   return Expr::ICmpLe;
    case Token::Type::Geq:   return Expr::ICmpGe;
    case Token::Type::Add:   return Expr::IAdd;
    case Token::Type::Sub:   return Expr::ISub;
    case Token::Type::Mul:   return Expr::IMul;
    case Token::Type::Div:   return Expr::IDiv;
    case Token::Type::Colon: return Expr::IAssign;
    default: return Expr::IEndOf;
    }
}

static int parse_expr_impl(ParseContext& ctx, Expr& result, float minbp, int l);

}

namespace shmy {

Expr Expr::from_string(const std::string& expr) {
    const auto temp = expr + ")";
    auto base = Expr{};
    auto ctx = detail::ParseContext{ detail::Lexer(temp.c_str()), {} };
    parse_expr_impl(ctx, base, 0.f, 1);
    base.bytecode.push_back(Instr::IEndOf);
    return base;
}

}

namespace shmy::detail {


#define EH_PROP(f) do { if (const auto _err = (f)) return _err; } while (0)


enum Error : int {
    OK = 0,
    INVALID_ATOM,
    UNCLOSED_PAREN,
    INVALID_OPERATOR,
    INVALID_ASSIGN,
};


static int parse_value(ParseContext& ctx, Expr& result) {
    Token n = ctx.lexer.next();
    switch (n.type) {
    case Token::Type::Ident: {
        uint32_t ref = 0;
        if (ctx.s_LUT.find(n.value) == ctx.s_LUT.end()) {
            ref = (uint32_t)result.idents.size();
            ctx.s_LUT[n.value] = ref;
            result.idents.emplace_back(n.value);
        } else {
            ref = ctx.s_LUT[n.value];
        }
        result.bytecode.push_back(Expr::IPushV);
        const auto idx = result.bytecode.size();
        result.bytecode.resize(result.bytecode.size() + 4);
        memcpy(result.bytecode.data() + idx, &ref, sizeof(uint32_t));
        break; }

    case Token::Type::IntLit: {
        const auto i = n.value;
        uint64_t v = 0;
        for (const char c : i) {
            v *= 10;
            v += (uint64_t)(c - '0');
        }
        result.bytecode.push_back(Expr::IPushC);
        const auto idx = result.bytecode.size();
        result.bytecode.resize(result.bytecode.size() + 8);
        memcpy(result.bytecode.data() + idx, &v, sizeof(uint64_t));
        break; }

    default:
        return INVALID_ATOM;
    }
    return OK;
}

static int parse_expr_impl(ParseContext& ctx, Expr& result, float minbp, int l) {
    Token n = ctx.lexer.peek();
    switch (n.type) {
    case Token::Type::LParen:
        ctx.lexer.next();
        EH_PROP(parse_expr_impl(ctx, result, 0, l + 1));
        n = ctx.lexer.next();
        if (n.type != Token::Type::RParen) return UNCLOSED_PAREN;
        if (l == 0) return 0;
        break;

    case Token::Type::Lnot: {
        ctx.lexer.next();
        EH_PROP(parse_expr_impl(ctx, result, 0, l));
        result.bytecode.push_back(Expr::ILogNot);
        break; }

    case Token::Type::Ident: case Token::Type::IntLit:
        EH_PROP(parse_value(ctx, result));
        break;

    default:
        return INVALID_ATOM;
    }

    while (true) {
        const Token op = ctx.lexer.peek();
        if (op.type == Token::Type::RParen) break;
        const struct Assoc bindstr = binding(op);
        if (bindstr.lhs < 0.f) return INVALID_OPERATOR;
        if (bindstr.lhs < minbp) break;
        ctx.lexer.next();
        EH_PROP(parse_expr_impl(ctx, result, bindstr.rhs, l));
        result.bytecode.push_back(opcode(op));
        // if (opcode(op) == Expr::IAssign && (/* last lhs tok */) != Token::Type::Ident) {
        //     return INVALID_ASSIGN;
        // }
    }

    return OK;
}

}
