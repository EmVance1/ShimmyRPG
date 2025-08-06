#include "pch.h"
#include "flag_expr.h"
#include "flags.h"


static std::random_device RD;
static std::mt19937 RNG(RD());


namespace shmy {

FlagExpr FlagExpr::binary(FlagExpr&& _lhs, FlagExpr&& _rhs, Operation _op) {
    FlagExpr result;
    result.lhs = new FlagExpr(std::move(_lhs));
    result.rhs = new FlagExpr(std::move(_rhs));
    result.op = _op;
    return result;
}
FlagExpr FlagExpr::unary(FlagExpr&& _lhs, Operation _op) {
    FlagExpr result;
    result.lhs = new FlagExpr(std::move(_lhs));
    result.op = _op;
    return result;
}
FlagExpr::FlagExpr(uint32_t _num, Operation _op)
    : op(_op), num(_num)
{}
FlagExpr::FlagExpr(std::string&& _ident, Operation _op)
    : op(_op), ident(std::move(_ident))
{}
FlagExpr::FlagExpr(std::string&& _id, uint32_t _mod, Operation _op)
    : op(_op), num(_mod), ident(std::move(_id))
{}

FlagExpr::FlagExpr(const FlagExpr& other) :
    lhs(other.lhs ? new FlagExpr(*other.lhs) : nullptr),
    rhs(other.rhs ? new FlagExpr(*other.rhs) : nullptr),
    op(other.op), num(other.num), ident(other.ident)
{}
FlagExpr::FlagExpr(FlagExpr&& other)
    : lhs(other.lhs), rhs(other.rhs), op(other.op), num(other.num), ident(std::move(other.ident))
{
    other.lhs = nullptr;
    other.rhs = nullptr;
}
FlagExpr::~FlagExpr() {
    delete lhs;
    delete rhs;
}

FlagExpr& FlagExpr::operator=(const FlagExpr& other) {
    this->~FlagExpr();
    lhs = other.lhs ? new FlagExpr(*other.lhs) : nullptr;
    rhs = other.rhs ? new FlagExpr(*other.rhs) : nullptr;
    op = other.op;
    num = other.num;
    ident = other.ident;
    return *this;
}
FlagExpr& FlagExpr::operator=(FlagExpr&& other) {
    this->~FlagExpr();
    lhs = other.lhs;
    rhs = other.rhs;
    op = other.op;
    num = other.num;
    ident = std::move(other.ident);
    other.lhs = nullptr;
    other.rhs = nullptr;
    return *this;
}

FlagExpr FlagExpr::Or(FlagExpr&& lhs, FlagExpr&& rhs) {
    return FlagExpr::binary(std::move(lhs), std::move(rhs), Operation::Or);
}
FlagExpr FlagExpr::And(FlagExpr&& lhs, FlagExpr&& rhs) {
    return FlagExpr::binary(std::move(lhs), std::move(rhs), Operation::And);
}
FlagExpr FlagExpr::Eq(FlagExpr&& lhs, FlagExpr&& rhs) {
    return FlagExpr::binary(std::move(lhs), std::move(rhs), Operation::Eq);
}
FlagExpr FlagExpr::Ne(FlagExpr&& lhs, FlagExpr&& rhs) {
    return FlagExpr::binary(std::move(lhs), std::move(rhs), Operation::Ne);
}
FlagExpr FlagExpr::Lt(FlagExpr&& lhs, FlagExpr&& rhs) {
    return FlagExpr::binary(std::move(lhs), std::move(rhs), Operation::Lt);
}
FlagExpr FlagExpr::Gt(FlagExpr&& lhs, FlagExpr&& rhs) {
    return FlagExpr::binary(std::move(lhs), std::move(rhs), Operation::Gt);
}
FlagExpr FlagExpr::Le(FlagExpr&& lhs, FlagExpr&& rhs) {
    return FlagExpr::binary(std::move(lhs), std::move(rhs), Operation::Le);
}
FlagExpr FlagExpr::Ge(FlagExpr&& lhs, FlagExpr&& rhs) {
    return FlagExpr::binary(std::move(lhs), std::move(rhs), Operation::Ge);
}
FlagExpr FlagExpr::Not(FlagExpr&& val) {
    return FlagExpr::unary(std::move(val), Operation::Not);
}
FlagExpr FlagExpr::Number(uint32_t num) {
    return FlagExpr{ num, Operation::Number };
}
FlagExpr FlagExpr::Identifier(std::string&& ident) {
    return FlagExpr{ std::move(ident), Operation::Identifier };
}

FlagExpr FlagExpr::True()  { return FlagExpr{ 1, Operation::Number }; }
FlagExpr FlagExpr::False() { return FlagExpr{ 0, Operation::Number }; }

FlagExpr FlagExpr::Once() {
    return FlagExpr{ "", Operation::Once };
}
FlagExpr FlagExpr::Random(std::string&& id, uint32_t mod) {
    return FlagExpr{ std::move(id), mod, Operation::Random };
}


int FlagExpr::evaluate() const {
    switch (op){
    case Operation::Or:
        return (int)((bool)lhs->evaluate() || (bool)rhs->evaluate());
    case Operation::And:
        return (int)((bool)lhs->evaluate() && (bool)rhs->evaluate());
    case Operation::Eq:
        return (int)(lhs->evaluate() == rhs->evaluate());
    case Operation::Ne:
        return (int)(lhs->evaluate() != rhs->evaluate());
    case Operation::Lt:
        return (int)(lhs->evaluate() <  rhs->evaluate());
    case Operation::Gt:
        return (int)(lhs->evaluate() >  rhs->evaluate());
    case Operation::Le:
        return (int)(lhs->evaluate() <= rhs->evaluate());
    case Operation::Ge:
        return (int)(lhs->evaluate() >= rhs->evaluate());
    case Operation::Not:
        return (int)(!((bool)lhs->evaluate()));
    case Operation::Number:
        return num;
    case Operation::Identifier:
        return FlagTable::get_flag(ident, true);
    case Operation::Once:
        return (int)FlagTable::Never;
    case Operation::Random:
        if (num != 0) {
            auto dist = std::uniform_int_distribution<uint32_t>(1, num);
            FlagTable::set_flag(ident, dist(RNG) - 1, false);
        }
        return FlagTable::get_flag(ident, true);
    }
    return 0;
}

std::ostream& operator<<(std::ostream& stream, const FlagExpr& value) {
    switch (value.op){
    case FlagExpr::Operation::Or:
        return stream << "(" << *value.lhs << "||" << *value.rhs << ")";
    case FlagExpr::Operation::And:
        return stream << "(" << *value.lhs << "&&" << *value.rhs << ")";
    case FlagExpr::Operation::Eq:
        return stream << "(" << *value.lhs << "==" << *value.rhs << ")";
    case FlagExpr::Operation::Ne:
        return stream << "(" << *value.lhs << "!=" << *value.rhs << ")";
    case FlagExpr::Operation::Lt:
        return stream << "(" << *value.lhs << "<"  << *value.rhs << ")";
    case FlagExpr::Operation::Gt:
        return stream << "(" << *value.lhs << ">"  << *value.rhs << ")";
    case FlagExpr::Operation::Le:
        return stream << "(" << *value.lhs << "<=" << *value.rhs << ")";
    case FlagExpr::Operation::Ge:
        return stream << "(" << *value.lhs << ">=" << *value.rhs << ")";
    case FlagExpr::Operation::Not:
        return stream << "!(" << *value.lhs << ")";
    case FlagExpr::Operation::Number:
        return stream << value.num;
    case FlagExpr::Operation::Identifier:
        return stream << value.ident;
    case FlagExpr::Operation::Once:
        return stream << "once";
    case FlagExpr::Operation::Random:
        if (value.num == 0) {
            return stream << value.ident;
        } else {
            return stream << value.ident << "(" << value.num << ")";
        }
    }
    return stream;
}

bool operator==(const FlagExpr& a, const FlagExpr& b) {
    return a.lhs == b.lhs &&
           a.rhs == b.rhs &&
           a.op == b.op &&
           a.num == b.num &&
           a.ident == b.ident;
}

}
