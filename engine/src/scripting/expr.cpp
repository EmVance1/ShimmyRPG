#include "pch.h"
#include "scripting/expr.h"


namespace shmy {


Expr Expr::True()  { return { Expr::ByteCode{ Expr::CTrue },  {} }; }
Expr Expr::False() { return { Expr::ByteCode{ Expr::CFalse }, {} }; }
Expr Expr::Constant(int64_t n) {
    auto result = Expr{};
    result.bytecode = Expr::ByteCode{ Expr::IPushC, 0, 0, 0, 0, 0, 0, 0, 0, Expr::IEndOf };
    memcpy(result.bytecode.data()+1, &n, sizeof(int64_t));
    return result;
}
Expr Expr::Variable(std::string&& id) {
    auto result = Expr{};
    result.bytecode = Expr::ByteCode{ Expr::IPushV, 0, 0, 0, 0, 0, 0, 0, 0, Expr::IEndOf };
    result.idents.push_back(id);
    return result;
}


int64_t Expr::evaluate(Callback ctx) const {
    return evaluate(bytecode.data(), idents, ctx);
}

int64_t Expr::evaluate(const uint8_t* bytecode, const std::vector<std::string>& idents, Callback ctx) {
    const uint8_t* pc = bytecode;
    int64_t stack[128] = { 0 };
    int64_t* sp = stack;
    bool strict = false;

#define POP(T) ((T)*(--sp))
#define PUSH(v) do { *(sp++) = (v); } while (0)

    while (true) {
        switch ((Instr)(*(pc++))) {

        case Instr::IEndOf:
            return stack[0];
        case Instr::IStrict:
            strict = true;
            break;

        case Instr::CTrue:
            return 1;
        case Instr::CFalse:
            return 0;

        case Instr::IPushC: {
            int64_t c;
            memcpy(&c, pc, sizeof(int64_t));
            PUSH((int64_t)c);
            pc += 8;
            break; }
        case Instr::IPushV: {
            uint32_t v;
            memcpy(&v, pc, sizeof(uint32_t));
            PUSH((int64_t)(*ctx(idents[v].c_str(), true)));
            pc += 4;
            break; }
        case Instr::IPushK: {
            uint32_t v;
            memcpy(&v, pc, sizeof(uint32_t));
            PUSH((int64_t)v);
            pc += 4;
            break; }

        case Instr::ICmpEq: {
            const int64_t r = POP(int64_t);
            const int64_t l = POP(int64_t);
            PUSH(l == r);
            break; }
        case Instr::ICmpNe: {
            const int64_t r = POP(int64_t);
            const int64_t l = POP(int64_t);
            PUSH(l != r);
            break; }
        case Instr::ICmpLt: {
            const int64_t r = POP(int64_t);
            const int64_t l = POP(int64_t);
            PUSH(l < r);
            break; }
        case Instr::ICmpGt: {
            const int64_t r = POP(int64_t);
            const int64_t l = POP(int64_t);
            PUSH(l > r);
            break; }
        case Instr::ICmpLe: {
            const int64_t r = POP(int64_t);
            const int64_t l = POP(int64_t);
            PUSH(l <= r);
            break; }
        case Instr::ICmpGe: {
            const int64_t r = POP(int64_t);
            const int64_t l = POP(int64_t);
            PUSH(l >= r);
            break; }
        case Instr::ILogNot: {
            const int64_t v = POP(int64_t);
            PUSH(!v);
            break; }
        case Instr::ILogOr: {
            const int64_t r = POP(int64_t);
            const int64_t l = POP(int64_t);
            PUSH(l || r);
            break; }
        case Instr::ILogAnd: {
            const int64_t r = POP(int64_t);
            const int64_t l = POP(int64_t);
            PUSH(l && r);
            break; }

        case Instr::IAssign: {
            const uint64_t v = POP(uint64_t);
            const uint32_t k = POP(uint32_t);
            *ctx(idents[(size_t)k].c_str(), false) = v;
            PUSH((int64_t)v);
            break; }
        case Instr::ISetV: {
            const uint64_t v = POP(uint64_t);
            const uint32_t k = POP(uint32_t);
            *ctx(idents[(size_t)k].c_str(), strict) = v;
            break; }
        case Instr::IAddV: {
            const int64_t v  = POP(int64_t);
            const uint32_t k = POP(uint32_t);
            *ctx(idents[(size_t)k].c_str(), strict) += (uint64_t)v; // subtraction safe because unsigned wrapping?
            break; }
        }
    }
}


std::ostream& operator<<(std::ostream& stream, const Expr& val) {
    const uint8_t* pc = &val.bytecode[0];
    stream << "(" << val.bytecode.size() << ")[";

    while (true) {
        switch ((uint8_t)(*(pc++))) {
        case Expr::Instr::IEndOf:
            return stream << " END ]";
        case Expr::Instr::IStrict:
            stream << " STRICT:";
            break;

        case Expr::Instr::CTrue:
            return stream << " TRUE ]";
        case Expr::Instr::CFalse:
            return stream << " FALSE ]";

        case Expr::Instr::IPushC: {
            int64_t c;
            memcpy(&c, pc, sizeof(int64_t));
            stream << " push(" << c << "),";
            pc += 8;
            break; }
        case Expr::Instr::IPushV: {
            uint32_t v;
            memcpy(&v, pc, sizeof(uint32_t));
            stream << " push(" << val.idents[(size_t)v] << "),";
            pc += 4;
            break; }
        case Expr::Instr::IPushK: {
            uint32_t k;
            memcpy(&k, pc, sizeof(uint32_t));
            stream << " pushkey(" << val.idents[(size_t)k] << "),";
            pc += 4;
            break; }
        case Expr::Instr::ICmpEq:
            stream << " ==,";
            break;
        case Expr::Instr::ICmpNe:
            stream << " ==,";
            break;
        case Expr::Instr::ICmpLt:
            stream << " <,";
            break;
        case Expr::Instr::ICmpGt:
            stream << " >,";
            break;
        case Expr::Instr::ICmpLe:
            stream << " <=,";
            break;
        case Expr::Instr::ICmpGe:
            stream << " >=,";
            break;
        case Expr::Instr::ILogNot:
            stream << " !,";
            break;
        case Expr::Instr::ILogAnd:
            stream << " &&,";
            break;
        case Expr::Instr::ILogOr:
            stream << " ||,";
            break;
        case Expr::Instr::IAssign:
            stream << " assign,";
            break;
        case Expr::Instr::ISetV:
            stream << " setv,";
            break;
        case Expr::Instr::IAddV:
            stream << " addv,";
            break;
        }
    }
}

bool operator==(const Expr& a, const Expr& b) {
    return a.bytecode == b.bytecode &&
           a.idents == b.idents;
}
bool operator!=(const Expr& a, const Expr& b) {
    return !(a == b);
}

}
