#include "pch.h"
#include "scripting/expr.h"
#include "scripting/lexer.h"


namespace shmy {

using namespace detail;

struct ExprContext {
    Lexer* lexer;
    std::vector<std::string> identifiers;
};

static void parse_or  (ExprContext& ctx, Expr& base, Token& next);
static void parse_and (ExprContext& ctx, Expr& base, Token& next);
static void parse_eq  (ExprContext& ctx, Expr& base, Token& next);
static void parse_cmp (ExprContext& ctx, Expr& base, Token& next);
static void parse_unit(ExprContext& ctx, Expr& base, Token& next);

Expr parse_flag_expr(Lexer& lexer) {
    auto ctx = ExprContext{ &lexer, {} };
    auto base = Expr{};
    auto next = *lexer.next();
    parse_or(ctx, base, next);
    base.bytecode.push_back(Expr::IEndOf);
    return base;
}

Expr Expr::from_string(const std::string& expr) {
    const auto temp = expr + ")";
    auto lexer = Lexer(temp);
    return parse_flag_expr(lexer);
}


static void parse_or(ExprContext& ctx, Expr& base, Token& next) {
    parse_and(ctx, base, next);

    while (true) {
        switch (next.type) {
        case TokenType::LogOr: {
            next = *ctx.lexer->next();
            parse_and(ctx, base, next);
            base.bytecode.push_back(Expr::ILogOr);
            break; }
        case TokenType::CloseParen:
            return;
        default:
            throw std::runtime_error("error parsing 'or' expression");
        }
    }
}

static void parse_and(ExprContext& ctx, Expr& base, Token& next) {
    parse_eq(ctx, base, next);

    while (true) {
        switch (next.type) {
        case TokenType::LogAnd: {
            next = *ctx.lexer->next();
            parse_eq(ctx, base, next);
            base.bytecode.push_back(Expr::ILogAnd);
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

static void parse_eq(ExprContext& ctx, Expr& base, Token& next) {
    parse_cmp(ctx, base, next);

    while (true) {
        switch (next.type) {
        case TokenType::EqualEq: {
            next = *ctx.lexer->next();
            parse_cmp(ctx, base, next);
            base.bytecode.push_back(Expr::ICmpEq);
            break; }
        case TokenType::BangEq: {
            next = *ctx.lexer->next();
            parse_cmp(ctx, base, next);
            base.bytecode.push_back(Expr::ICmpNe);
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

static void parse_cmp(ExprContext& ctx, Expr& base, Token& next) {
    parse_unit(ctx, base, next);

    while (true) {
        switch (next.type) {
        case TokenType::LessThan: {
            next = *ctx.lexer->next();
            parse_unit(ctx, base, next);
            base.bytecode.push_back(Expr::ICmpLt);
            break; }
        case TokenType::GreaterThan: {
            next = *ctx.lexer->next();
            parse_unit(ctx, base, next);
            base.bytecode.push_back(Expr::ICmpGt);
            break; }
        case TokenType::LessEq: {
            next = *ctx.lexer->next();
            parse_unit(ctx, base, next);
            base.bytecode.push_back(Expr::ICmpLe);
            break; }
        case TokenType::GreaterEq: {
            next = *ctx.lexer->next();
            parse_unit(ctx, base, next);
            base.bytecode.push_back(Expr::ICmpGe);
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

static void parse_unit(ExprContext& ctx, Expr& base, Token& next) {
    switch (next.type) {
    case TokenType::IntLiteral: {
        const auto i = next.val;
        next = *ctx.lexer->next();
        const auto n = std::atoll(i.c_str());

        base.bytecode.push_back(Expr::IPushC);
        const auto idx = base.bytecode.size();
        base.bytecode.resize(base.bytecode.size() + 8);
        memcpy(base.bytecode.data() + idx, &n, sizeof(int64_t));
        break; }

    case TokenType::Identifier: {
        const auto id = next.val;
        next = *ctx.lexer->next();
        const auto n = base.idents.size();
        base.idents.push_back(std::move(id));

        if (next.type == TokenType::Colon) {
            base.bytecode.push_back(Expr::IPushK);
            const auto idx = base.bytecode.size();
            base.bytecode.resize(idx + sizeof(size_t));
            memcpy(base.bytecode.data() + idx, &n, sizeof(size_t));

            next = *ctx.lexer->next();
            parse_unit(ctx, base, next);
            base.bytecode.push_back(Expr::IAssign);
        } else {
            base.bytecode.push_back(Expr::IPushV);
            const auto idx = base.bytecode.size();
            base.bytecode.resize(idx + sizeof(size_t));
            memcpy(base.bytecode.data() + idx, &n, sizeof(size_t));
        }
        break; }

    case TokenType::Bang: {
        next = *ctx.lexer->next();
        parse_unit(ctx, base, next);
        base.bytecode.push_back(Expr::ILogNot);
        return; }

    case TokenType::OpenParen: {
        next = *ctx.lexer->next();
        parse_or(ctx, base, next);
        if (next.type == TokenType::CloseParen) {
            next = *ctx.lexer->next();
            return;
        } else {
            throw std::runtime_error("should be unreachable");
        }
        break; }

    default:
        throw std::runtime_error("error parsing unary expression");
    }
}

}
