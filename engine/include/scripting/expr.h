#pragma once
#include <cstdint>
#include <string>
#include <vector>


namespace shmy {

struct Expr {
    enum Instr : uint8_t {
        IEndOf,
        IStrict,

        CTrue,
        CFalse,

        IPushC,
        IPushV,
        IPushK,
        ICmpEq,
        ICmpNe,
        ICmpLt,
        ICmpGt,
        ICmpLe,
        ICmpGe,
        ILogNot,
        ILogOr,
        ILogAnd,
        IAssign,
        ISetV,
        IAddV,
    };
    using ByteCode = std::vector<uint8_t>;
    using IdTable = std::vector<std::string>;
    using Callback = uint64_t*(*)(const char* key, bool strict);

    ByteCode bytecode;
    IdTable idents;

    static Expr True();
    static Expr False();
    static Expr Constant(int64_t n);
    static Expr Variable(std::string&& id);
    static Expr from_string(const std::string& expr);

    static int64_t evaluate(const uint8_t* bytecode, const std::vector<std::string>& idents, Callback ctx);
    int64_t evaluate(Callback ctx) const;
};

std::ostream& operator<<(std::ostream& stream, const Expr& val);
bool operator==(const Expr& lhs, const Expr& rhs);
bool operator!=(const Expr& lhs, const Expr& rhs);

}
