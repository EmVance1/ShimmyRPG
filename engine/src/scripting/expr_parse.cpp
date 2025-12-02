#include "pch.h"
#include "scripting/expr.h"
#include "scripting/speech/lexer.h"
#include "scripting/speech/ctxt.h"


namespace shmy { namespace speech { namespace detail {

struct ParseContext;

void append_flag_expr(detail::ParseContext& ctx, Expr::ByteCode& base, Expr::IdTable& table);

static void parse_or  (detail::ParseContext& ctx, Expr::ByteCode& base, Expr::IdTable& table);
static void parse_and (detail::ParseContext& ctx, Expr::ByteCode& base, Expr::IdTable& table);
static void parse_eq  (detail::ParseContext& ctx, Expr::ByteCode& base, Expr::IdTable& table);
static void parse_cmp (detail::ParseContext& ctx, Expr::ByteCode& base, Expr::IdTable& table);
static void parse_unit(detail::ParseContext& ctx, Expr::ByteCode& base, Expr::IdTable& table);

} }

Expr Expr::from_string(const std::string& expr) {
    const auto temp = expr + ")";
    auto base = Expr{};
    auto ctx = speech::detail::ParseContext{
        speech::detail::Lexer(temp),
        nullptr,
    };
    speech::detail::append_flag_expr(ctx, base.bytecode, base.idents);
    return base;
}


namespace speech { namespace detail {


void append_flag_expr(ParseContext& ctx, Expr::ByteCode& base, Expr::IdTable& table) {
    parse_or(ctx, base, table);
    base.push_back(Expr::IEndOf);
}


static void parse_or(ParseContext& ctx, Expr::ByteCode& base, Expr::IdTable& table) {
    parse_and(ctx, base, table);

    while (true) {
        switch (ctx.lexer.peek()->type) {
        case TokenType::LogOr: {
            ctx.lexer.next();
            parse_and(ctx, base, table);
            base.push_back(Expr::ILogOr);
            break; }
        case TokenType::CloseParen:
            return;
        default:
            throw std::runtime_error("error parsing 'or' expression");
        }
    }
}

static void parse_and(ParseContext& ctx, Expr::ByteCode& base, Expr::IdTable& table) {
    parse_eq(ctx, base, table);

    while (true) {
        switch (ctx.lexer.peek()->type) {
        case TokenType::LogAnd: {
            ctx.lexer.next();
            parse_eq(ctx, base, table);
            base.push_back(Expr::ILogAnd);
            break; }
        case TokenType::LogOr:
            return;
        case TokenType::CloseParen:
            return;
        default:
            throw std::runtime_error("error parsing 'and' expression");
        }
    }
}

static void parse_eq(ParseContext& ctx, Expr::ByteCode& base, Expr::IdTable& table) {
    parse_cmp(ctx, base, table);

    while (true) {
        switch (ctx.lexer.peek()->type) {
        case TokenType::EqualEq: {
            ctx.lexer.next();
            parse_cmp(ctx, base, table);
            base.push_back(Expr::ICmpEq);
            break; }
        case TokenType::BangEq: {
            ctx.lexer.next();
            parse_cmp(ctx, base, table);
            base.push_back(Expr::ICmpNe);
            break; }
        case TokenType::LogAnd: case TokenType::LogOr:
            return;
        case TokenType::CloseParen:
            return;
        default:
            throw std::runtime_error("error parsing 'eq' expression");
        }
    }
}

static void parse_cmp(ParseContext& ctx, Expr::ByteCode& base, Expr::IdTable& table) {
    parse_unit(ctx, base, table);

    while (true) {
        switch (ctx.lexer.peek()->type) {
        case TokenType::LessThan: {
            ctx.lexer.next();
            parse_unit(ctx, base, table);
            base.push_back(Expr::ICmpLt);
            break; }
        case TokenType::GreaterThan: {
            ctx.lexer.next();
            parse_unit(ctx, base, table);
            base.push_back(Expr::ICmpGt);
            break; }
        case TokenType::LessEq: {
            ctx.lexer.next();
            parse_unit(ctx, base, table);
            base.push_back(Expr::ICmpLe);
            break; }
        case TokenType::GreaterEq: {
            ctx.lexer.next();
            parse_unit(ctx, base, table);
            base.push_back(Expr::ICmpGe);
            break; }
        case TokenType::LogAnd: case TokenType::LogOr: case TokenType::EqualEq: case TokenType::BangEq:
            return;
        case TokenType::CloseParen:
            return;
        default:
            throw std::runtime_error("error parsing 'cmp' expression");
        }
    }
}

static void parse_unit(ParseContext& ctx, Expr::ByteCode& base, Expr::IdTable& table) {
    auto tok = ctx.lexer.next();

    switch (tok->type) {
    case TokenType::IntLiteral: {
        const auto i = tok->val;
        const auto n = std::atoll(i.c_str());

        base.push_back(Expr::IPushC);
        const auto idx = base.size();
        base.resize(base.size() + 8);
        memcpy(base.data() + idx, &n, sizeof(int64_t));
        break; }

    case TokenType::Identifier: {
        uint32_t ref = 0;
        if (ctx.s_LUT.find(tok->val) == ctx.s_LUT.end()) {
            ref = (uint32_t)table.size();
            ctx.s_LUT[tok->val] = ref;
            table.emplace_back(std::move(tok->val));
        } else {
            ref = ctx.s_LUT[tok->val];
        }

        if (ctx.lexer.peek()->type == TokenType::Colon) {
            base.push_back(Expr::IPushK);
            const auto idx = base.size();
            base.resize(idx + sizeof(uint32_t));
            memcpy(base.data() + idx, &ref, sizeof(uint32_t));

            ctx.lexer.next();
            parse_unit(ctx, base, table);
            base.push_back(Expr::IAssign);
        } else {
            base.push_back(Expr::IPushV);
            const auto idx = base.size();
            base.resize(idx + sizeof(uint32_t));
            memcpy(base.data() + idx, &ref, sizeof(uint32_t));
        }
        break; }

    case TokenType::Bang: {
        parse_unit(ctx, base, table);
        base.push_back(Expr::ILogNot);
        return; }

    case TokenType::OpenParen: {
        parse_or(ctx, base, table);
        if (ctx.lexer.next()->type == TokenType::CloseParen) {
            return;
        } else {
            throw std::runtime_error("should be unreachable");
        }
        break; }

    default:
        throw std::runtime_error("error parsing unary expression");
    }
}

} } }
