#include "pch.h"
#include "flag_expr.h"
#include "flags.h"


static std::random_device RD;
static std::mt19937 RNG(RD());


int FlagExpr::evaluate() const {
    switch (op){
    case FlagExpr::Type::Or:
        return (int)((bool)left->evaluate() || (bool)right->evaluate());
    case FlagExpr::Type::And:
        return (int)((bool)left->evaluate() && (bool)right->evaluate());
    case FlagExpr::Type::Eq:
        return (int)(left->evaluate() == right->evaluate());
    case FlagExpr::Type::Ne:
        return (int)(left->evaluate() != right->evaluate());
    case FlagExpr::Type::Lt:
        return (int)(left->evaluate() <  right->evaluate());
    case FlagExpr::Type::Gt:
        return (int)(left->evaluate() >  right->evaluate());
    case FlagExpr::Type::Le:
        return (int)(left->evaluate() <= right->evaluate());
    case FlagExpr::Type::Ge:
        return (int)(left->evaluate() >= right->evaluate());
    case FlagExpr::Type::Not:
        return (int)(!((bool)left->evaluate()));
    case FlagExpr::Type::Default:
        return 1;
    case FlagExpr::Type::Value:
        return value;
    case FlagExpr::Type::Identifier:
        return FlagTable::get_flag(name, true);
    case FlagExpr::Type::Once:
        if (!FlagTable::Once) {
            FlagTable::Once = true;
            return 1;
        }
        return 0;
    case FlagExpr::Type::Random:
        if (value != 0) {
            auto dist = std::uniform_int_distribution<uint32_t>(1, value);
            FlagTable::set_flag(name, dist(RNG) - 1, false);
        }
        return FlagTable::get_flag(name, true);
    }
    return 0;
}

std::ostream& operator<<(std::ostream& stream, const FlagExpr& value) {
    switch (value.op){
    case FlagExpr::Type::Or:
        return stream << "(" << *value.left << "||" << *value.right << ")";
    case FlagExpr::Type::And:
        return stream << "(" << *value.left << "&&" << *value.right << ")";
    case FlagExpr::Type::Eq:
        return stream << "(" << *value.left << "==" << *value.right << ")";
    case FlagExpr::Type::Ne:
        return stream << "(" << *value.left << "!=" << *value.right << ")";
    case FlagExpr::Type::Lt:
        return stream << "(" << *value.left << "<"  << *value.right << ")";
    case FlagExpr::Type::Gt:
        return stream << "(" << *value.left << ">"  << *value.right << ")";
    case FlagExpr::Type::Le:
        return stream << "(" << *value.left << "<=" << *value.right << ")";
    case FlagExpr::Type::Ge:
        return stream << "(" << *value.left << ">=" << *value.right << ")";
    case FlagExpr::Type::Not:
        return stream << "!(" << *value.left << ")";
    case FlagExpr::Type::Value:
        return stream << value.value;
    case FlagExpr::Type::Identifier:
        return stream << value.name;
    case FlagExpr::Type::Default:
        return stream << "default";
    case FlagExpr::Type::Once:
        return stream << "once";
    case FlagExpr::Type::Random:
        if (value.value == 0) {
            return stream << value.name;
        } else {
            return stream << value.name << "(" << value.value << ")";
        }
    }
    return stream;
}

bool operator==(const FlagExpr& a, const FlagExpr& b) {
    return a.left == b.left &&
           a.right == b.right &&
           a.op == b.op &&
           a.value == b.value &&
           a.name == b.name;
}

