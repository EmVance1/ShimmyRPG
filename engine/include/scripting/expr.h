#pragma once
#include <cstdint>
#include <string>
#include <vector>


namespace shmy {


struct Expr {
    enum Instr : uint8_t {
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
        IEndOf,
    };
    using ByteCode = std::vector<uint8_t>;
    using IdTable = std::vector<std::string>;

    ByteCode bytecode;
    IdTable idents;
    bool strict = true;

    static Expr Constant(int64_t n);
    static Expr Variable(std::string&& id);
    static Expr True();
    static Expr False();

    static Expr from_string(const std::string& expr);

    int64_t evaluate(uint64_t*(*ctx)(const char* key, bool strict)) const;
};

bool operator==(const Expr& a, const Expr& b);
std::ostream& operator<<(std::ostream& stream, const Expr& ex);


}
