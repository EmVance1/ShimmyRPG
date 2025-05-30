#include "pch.h"
#include "flag_expr.h"
#include "lexer.h"


FlagExpr parse_or(Lexer& lexer, Token& next);
FlagExpr parse_and(Lexer& lexer, Token& next);
FlagExpr parse_eq(Lexer& lexer, Token& next);
FlagExpr parse_cmp(Lexer& lexer, Token& next);
FlagExpr parse_unit(Lexer& lexer, Token& next);
FlagExpr parse_random(const std::string& id);


FlagExpr parse_or(Lexer& lexer, Token& next) {
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
            throw std::exception("error parsing 'or' expression");
        }
    }
}

FlagExpr parse_and(Lexer& lexer, Token& next) {
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
            throw std::exception("error parsing 'and' expression");
        }
    }
}

FlagExpr parse_eq(Lexer& lexer, Token& next) {
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
            throw std::exception("error parsing 'eq' expression");
        }
    }
}

FlagExpr parse_cmp(Lexer& lexer, Token& next) {
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
            throw std::exception("error parsing 'cmp' expression");
        }
    }
}

FlagExpr parse_unit(Lexer& lexer, Token& next) {
    switch (next.type) {
    case TokenType::Identifier: {
        auto id = next.val;
        next = *lexer.next();
        if (id == "default") {
            return FlagExpr::Default();
        } else if (id == "once") {
            return FlagExpr::Once();
        } else if (id.starts_with("rng_")) {
            return parse_random(id);
        } else {
            return FlagExpr::Identifier(std::move(id));
        }
        break; }
    case TokenType::IntLiteral: {
        auto i = next.val;
        next = *lexer.next();
        return FlagExpr::Value(std::atoi(i.c_str())); }
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
            throw std::exception("should be unreachable");
        }
        break; }
    default:
        throw std::exception("error parsing unary expression");
    }
}

FlagExpr parse_flag_expr(Lexer& lexer) {
    auto next = *lexer.next();
    return parse_or(lexer, next);
}

FlagExpr flagexpr_from_string(const std::string& expr) {
    const auto temp = expr + ")";
    auto lexer = Lexer(temp);
    return parse_flag_expr(lexer);
}



FlagExpr parse_random(const std::string& id) {
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

