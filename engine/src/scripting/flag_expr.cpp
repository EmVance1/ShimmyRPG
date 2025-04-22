#include "pch.h"
#include "flag_expr.h"
#include "flags.h"


int FlagExpr::evaluate() const {
    switch (op){
    case FlagExprType::Or:
        return (int)((bool)left->evaluate() || (bool)right->evaluate());
    case FlagExprType::And:
        return (int)((bool)left->evaluate() && (bool)right->evaluate());
    case FlagExprType::Eq:
        return (int)(left->evaluate() == right->evaluate());
    case FlagExprType::Ne:
        return (int)(left->evaluate() != right->evaluate());
    case FlagExprType::Lt:
        return (int)(left->evaluate() <  right->evaluate());
    case FlagExprType::Gt:
        return (int)(left->evaluate() >  right->evaluate());
    case FlagExprType::Le:
        return (int)(left->evaluate() <= right->evaluate());
    case FlagExprType::Ge:
        return (int)(left->evaluate() >= right->evaluate());
    case FlagExprType::Not:
        return (int)(!((bool)left->evaluate()));
    case FlagExprType::Value:
        return value;
    case FlagExprType::Identifier:
        return FlagTable::get_flag(name);
    }
    return 0;
}

std::ostream& operator<<(std::ostream& stream, const FlagExpr& value) {
    switch (value.op){
    case FlagExprType::Or:
        return stream << "(" << *value.left << "||" << *value.right << ")";
    case FlagExprType::And:
        return stream << "(" << *value.left << "&&" << *value.right << ")";
    case FlagExprType::Eq:
        return stream << "(" << *value.left << "==" << *value.right << ")";
    case FlagExprType::Ne:
        return stream << "(" << *value.left << "!=" << *value.right << ")";
    case FlagExprType::Lt:
        return stream << "(" << *value.left << "<"  << *value.right << ")";
    case FlagExprType::Gt:
        return stream << "(" << *value.left << ">"  << *value.right << ")";
    case FlagExprType::Le:
        return stream << "(" << *value.left << "<=" << *value.right << ")";
    case FlagExprType::Ge:
        return stream << "(" << *value.left << ">=" << *value.right << ")";
    case FlagExprType::Not:
        return stream << "!(" << *value.left << ")";
    case FlagExprType::Value:
        return stream << value.value;
    case FlagExprType::Identifier:
        return stream << value.name;
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

