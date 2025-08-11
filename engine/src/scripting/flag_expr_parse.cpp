#include "pch.h"
#include "flag_expr.h"
#include "lexer.h"


namespace shmy {

using namespace detail;

static FlagExpr parse_or(Lexer& lexer, Token& next);
static FlagExpr parse_and(Lexer& lexer, Token& next);
static FlagExpr parse_eq(Lexer& lexer, Token& next);
static FlagExpr parse_cmp(Lexer& lexer, Token& next);
static FlagExpr parse_unit(Lexer& lexer, Token& next);
static FlagExpr parse_random(const std::string& id);

FlagExpr parse_flag_expr(Lexer& lexer) {
    auto next = *lexer.next();
    return parse_or(lexer, next);
}

FlagExpr FlagExpr::from_string(const std::string& expr) {
    const auto temp = expr + ")";
    auto lexer = Lexer(temp);
    return parse_flag_expr(lexer);
}


static FlagExpr parse_or(Lexer& lexer, Token& next) {
    auto a = parse_and(lexer, next);

    while (true) {
        switch (next.type) {
        case TokenType::LogOr: {
            next = *lexer.next();
            auto b = parse_and(lexer, next);
            a = FlagExpr::Or(std::move(a), std::move(b));
            break; }
        case TokenType::CloseParen:
            return a;
        default:
            throw std::runtime_error("error parsing 'or' expression");
        }
    }
}

static FlagExpr parse_and(Lexer& lexer, Token& next) {
    auto a = parse_eq(lexer, next);

    while (true) {
        switch (next.type) {
        case TokenType::LogAnd: {
            next = *lexer.next();
            auto b = parse_eq(lexer, next);
            a = FlagExpr::And(std::move(a), std::move(b));
            break; }
        case TokenType::LogOr:
            return a;
        case TokenType::CloseParen:
            return a;
        default:
            throw std::runtime_error("error parsing 'and' expression");
        }
    }
}

static FlagExpr parse_eq(Lexer& lexer, Token& next) {
    auto a = parse_cmp(lexer, next);

    while (true) {
        switch (next.type) {
        case TokenType::EqualEq: {
            next = *lexer.next();
            auto b = parse_cmp(lexer, next);
            a = FlagExpr::Eq(std::move(a), std::move(b));
            break; }
        case TokenType::BangEq: {
            next = *lexer.next();
            auto b = parse_cmp(lexer, next);
            a = FlagExpr::Ne(std::move(a), std::move(b));
            break; }
        case TokenType::LogAnd: case TokenType::LogOr:
            return a;
        case TokenType::CloseParen:
            return a;
        default:
            throw std::runtime_error("error parsing 'eq' expression");
        }
    }
}

static FlagExpr parse_cmp(Lexer& lexer, Token& next) {
    auto a = parse_unit(lexer, next);

    while (true) {
        switch (next.type) {
        case TokenType::LessThan: {
            next = *lexer.next();
            auto b = parse_unit(lexer, next);
            a = FlagExpr::Lt(std::move(a), std::move(b));
            break; }
        case TokenType::GreaterThan: {
            next = *lexer.next();
            auto b = parse_unit(lexer, next);
            a = FlagExpr::Gt(std::move(a), std::move(b));
            break; }
        case TokenType::LessEq: {
            next = *lexer.next();
            auto b = parse_unit(lexer, next);
            a = FlagExpr::Le(std::move(a), std::move(b));
            break; }
        case TokenType::GreaterEq: {
            next = *lexer.next();
            auto b = parse_unit(lexer, next);
            a = FlagExpr::Ge(std::move(a), std::move(b));
            break; }
        case TokenType::LogAnd: case TokenType::LogOr: case TokenType::EqualEq: case TokenType::BangEq:
            return a;
        case TokenType::CloseParen:
            return a;
        default:
            throw std::runtime_error("error parsing 'cmp' expression");
        }
    }
}

/*
static std::pair<FlagExpr, bool> parse_post(Lexer& lexer, Token& next, FlagExpr&& inner) {
    switch (next.type) {
    case TokenType::Colon:
        next = *lexer.next();
        if (next.type == TokenType::Identifier) {
            auto name = next.val;
            next = *lexer.next();
            return FlagExpr::Assign(std::move(inner), FlagExpr::Func());
        } else {
            throw std::runtime_error("error parsing assign expression");
        }
    default:
        throw std::runtime_error("error parsing postfix expression");
    }
}
*/

static FlagExpr parse_unit(Lexer& lexer, Token& next) {
    switch (next.type) {
    case TokenType::IntLiteral: {
        auto i = next.val;
        next = *lexer.next();
        return FlagExpr::Number(std::atoi(i.c_str())); }
    case TokenType::Identifier: {
        auto id = next.val;
        next = *lexer.next();
        if (id == "default") {
            return FlagExpr::True();
        } else if (id == "once") {
            return FlagExpr::Once();
        } else if (id.starts_with("rng_")) {
            return parse_random(id);
        } else {
            /*
            auto [exp, found] = parse_post(lexer, next, FlagExpr::Identifier(std::move(id)));
            while (found) {
                auto [e2, f2] = parse_post(lexer, next, std::move(exp));
                exp = std::move(e2);
                found = f2;
            }
            */
            return FlagExpr::Identifier(std::move(id));
        }
        break; }
    case TokenType::Bang: {
        next = *lexer.next();
        auto val = parse_unit(lexer, next);
        return FlagExpr::Not(std::move(val)); }
    case TokenType::OpenParen: {
        next = *lexer.next();
        auto exp = parse_or(lexer, next);
        if (next.type == TokenType::CloseParen) {
            next = *lexer.next();
            return exp;
        } else {
            throw std::runtime_error("should be unreachable");
        }
        break; }
    default:
        throw std::runtime_error("error parsing unary expression");
    }
}

static FlagExpr parse_random(const std::string& id) {
    auto num = std::string("");
    auto i = 0;
    while (isdigit(id[id.size() - 1 - i])) {
        num.insert(num.begin() + i, id[id.size() - 1 - i]);
        i++;
    }
    if (i == 0) {
        auto val = id;
        return FlagExpr::Random(std::move(val), 0);
    } else {
        auto val = id.substr(0, id.size() - i);
        return FlagExpr::Random(std::move(val), std::atoi(num.c_str()));
    }
}

}
