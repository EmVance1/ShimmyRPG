#include "pch.h"
#include "scripting/expr.h"


namespace shmy {

Expr Expr::Constant(int64_t n) {
    auto bs = Expr::ByteCode{ Expr::IPushC, 0, 0, 0, 0, 0, 0, 0, 0, Expr::IEndOf };
    memcpy(bs.data() + 1, &n, sizeof(int64_t));
    return { bs, {} };
}

Expr Expr::Variable(std::string&& id) {
    auto result = Expr{};
    result.bytecode = Expr::ByteCode{ Expr::IPushV, 0, 0, 0, 0, 0, 0, 0, 0, Expr::IEndOf };
    result.idents.push_back(id);
    return result;
}

Expr Expr::True()  { return Expr::Constant(1); }
Expr Expr::False() { return Expr::Constant(0); }


int64_t Expr::evaluate(uint64_t*(*ctx)(const char* key, bool strict)) const {
    const uint8_t* pc = &bytecode[0];
    int64_t stack[128] = { 0 };
    int64_t* sp = stack;

#define POP() (*(--sp))
#define PUSH(v) do { *(sp++) = (v); } while (0)

    while (true) {
        switch ((Instr)(*(pc++))) {
        case Instr::IPushC: {
            int64_t c;
            memcpy(&c, pc, sizeof(int64_t));
            PUSH((int64_t)c);
            pc += 8;
            break; }
        case Instr::IPushV: {
            size_t v;
            memcpy(&v, pc, sizeof(size_t));
            PUSH((int64_t)(*ctx(idents[v].c_str(), true)));
            pc += 8;
            break; }
        case Instr::IPushK: {
            size_t v;
            memcpy(&v, pc, sizeof(size_t));
            PUSH((int64_t)v);
            pc += 8;
            break; }
        case Instr::ICmpEq: {
            const int64_t r = POP();
            const int64_t l = POP();
            PUSH(r == l);
            break; }
        case Instr::ICmpNe: {
            const int64_t r = POP();
            const int64_t l = POP();
            PUSH(r != l);
            break; }
        case Instr::ICmpLt: {
            const int64_t r = POP();
            const int64_t l = POP();
            PUSH(r < l);
            break; }
        case Instr::ICmpGt: {
            const int64_t r = POP();
            const int64_t l = POP();
            PUSH(r > l);
            break; }
        case Instr::ICmpLe: {
            const int64_t r = POP();
            const int64_t l = POP();
            PUSH(r <= l);
            break; }
        case Instr::ICmpGe: {
            const int64_t r = POP();
            const int64_t l = POP();
            PUSH(r >= l);
            break; }
        case Instr::ILogNot: {
            const int64_t v = POP();
            PUSH(!v);
            break; }
        case Instr::ILogOr: {
            const int64_t r = POP();
            const int64_t l = POP();
            PUSH(r || l);
            break; }
        case Instr::ILogAnd: {
            const int64_t r = POP();
            const int64_t l = POP();
            PUSH(r && l);
            break; }
        case Instr::IAssign: {
            const int64_t v = POP();
            const size_t k = (size_t)POP();
            *ctx(idents[k].c_str(), false) = (uint64_t)v;
            PUSH(v);
            break; }
        case Instr::ISetV: {
            const int64_t v = POP();
            const size_t k = (size_t)POP();
            *ctx(idents[k].c_str(), strict) = (uint64_t)v;
            break; }
        case Instr::IAddV: {
            const int64_t v = POP();
            const size_t k = (size_t)POP();
            *ctx(idents[k].c_str(), strict) += (uint64_t)v;
            break; }
        case Instr::IEndOf:
            return stack[0];
        }
    }
}

bool operator==(const Expr& a, const Expr& b) {
    return a.bytecode == b.bytecode &&
           a.idents == b.idents;
}

std::ostream& operator<<(std::ostream& stream, const Expr& ex) {
    const uint8_t* pc = &ex.bytecode[0];
    stream << "[";

    while (true) {
        switch ((uint8_t)(*(pc++))) {
        case Expr::Instr::IPushC: {
            int64_t c;
            memcpy(&c, pc, sizeof(int64_t));
            stream << " push(" << c << "),";
            pc += 8;
            break; }
        case Expr::Instr::IPushV: {
            size_t v;
            memcpy(&v, pc, sizeof(size_t));
            stream << " push(" << ex.idents[v] << "),";
            pc += 8;
            break; }
        case Expr::Instr::IPushK: {
            size_t k;
            memcpy(&k, pc, sizeof(size_t));
            stream << " pushkey(" << ex.idents[k] << "),";
            pc += 8;
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
        case Expr::Instr::IEndOf:
            stream << " end ]";
            return stream;
        }
    }
}

}
