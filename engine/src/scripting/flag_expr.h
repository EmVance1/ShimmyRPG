#pragma once
#include <string>


namespace shmy {

struct FlagExpr {
private:
    enum class Operation {
        Or,
        And,
        Eq,
        Ne,
        Lt,
        Gt,
        Le,
        Ge,
        Not,

        Number,
        Identifier,

        Once,
        Random,
    };

    FlagExpr* lhs = nullptr;
    FlagExpr* rhs = nullptr;
    Operation op = Operation::Number;
    uint32_t num = 0;
    std::string ident = "";

private:
    static FlagExpr binary(FlagExpr&& lhs, FlagExpr&& rhs, Operation op);
    static FlagExpr unary(FlagExpr&& lhs, Operation op);
    FlagExpr(uint32_t num, Operation op);
    FlagExpr(std::string&& ident, Operation op);
    FlagExpr(std::string&& id, uint32_t mod, Operation op);

public:
    FlagExpr() = default;
    FlagExpr(const FlagExpr& other);
    FlagExpr(FlagExpr&& other);
    ~FlagExpr();

    FlagExpr& operator=(const FlagExpr& other);
    FlagExpr& operator=(FlagExpr&& other);

    static FlagExpr Or(FlagExpr&& lhs, FlagExpr&& rhs);
    static FlagExpr And(FlagExpr&& lhs, FlagExpr&& rhs);
    static FlagExpr Eq(FlagExpr&& lhs, FlagExpr&& rhs);
    static FlagExpr Ne(FlagExpr&& lhs, FlagExpr&& rhs);
    static FlagExpr Lt(FlagExpr&& lhs, FlagExpr&& rhs);
    static FlagExpr Gt(FlagExpr&& lhs, FlagExpr&& rhs);
    static FlagExpr Le(FlagExpr&& lhs, FlagExpr&& rhs);
    static FlagExpr Ge(FlagExpr&& lhs, FlagExpr&& rhs);
    static FlagExpr Not(FlagExpr&& val);
    static FlagExpr Number(uint32_t num);
    static FlagExpr Identifier(std::string&& ident);

    static FlagExpr True();
    static FlagExpr False();

    static FlagExpr Once();
    static FlagExpr Random(std::string&& id, uint32_t mod);

    static FlagExpr from_string(const std::string& expr);

    int evaluate() const;

    friend bool operator==(const FlagExpr& a, const FlagExpr& b);
    friend std::ostream& operator<<(std::ostream& stream, const FlagExpr& value);
};

bool operator==(const FlagExpr& a, const FlagExpr& b);
std::ostream& operator<<(std::ostream& stream, const FlagExpr& value);

}
