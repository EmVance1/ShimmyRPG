#pragma once
#include <string>
#include <memory>


enum class FlagExprType {
    Or,
    And,
    Eq,
    Ne,
    Lt,
    Gt,
    Le,
    Ge,
    Not,

    Value,
    Identifier,

    Default,
    Once,
    Random,
};

struct FlagExpr {
    std::unique_ptr<FlagExpr> left;
    std::unique_ptr<FlagExpr> right;
    FlagExprType op;
    std::string name;
    uint32_t value;

    FlagExpr()
        : left(nullptr), right(nullptr), op(FlagExprType::Value), name(""), value(1)
    {}
    FlagExpr(const FlagExpr& other)
        : left(other.left ? std::make_unique<FlagExpr>(*other.left) : nullptr),
        right(other.right ? std::make_unique<FlagExpr>(*other.right) : nullptr),
        op(other.op), name(other.name), value(other.value)
    {}
    FlagExpr(FlagExpr&& other)
        : left(std::move(other.left)), right(std::move(other.right)), op(other.op), name(std::move(other.name)), value(other.value)
    {}
    FlagExpr(FlagExpr&& _left, FlagExpr&& _right, FlagExprType _op)
        : left(std::make_unique<FlagExpr>(std::move(_left))), right(std::make_unique<FlagExpr>(std::move(_right))), op(_op)
    {}
    FlagExpr(FlagExpr&& _val, FlagExprType _op)
        : left(std::make_unique<FlagExpr>(std::move(_val))), right(nullptr), op(_op)
    {}
    FlagExpr(std::string&& _val, FlagExprType _op)
        : left(nullptr), right(nullptr), op(_op), name(std::move(_val)), value(0)
    {}
    FlagExpr(uint32_t _val, FlagExprType _op)
        : left(nullptr), right(nullptr), op(_op), name(""), value(_val)
    {}
    FlagExpr(std::string&& _id, uint32_t _mod, FlagExprType _op)
        : left(nullptr), right(nullptr), op(_op), name(std::move(_id)), value(_mod)
    {}

    FlagExpr& operator=(const FlagExpr& other) {
        left = other.left ? std::make_unique<FlagExpr>(*other.left) : nullptr;
        right = other.right ? std::make_unique<FlagExpr>(*other.right) : nullptr;
        op = other.op;
        name = other.name;
        value = other.value;
        return *this;
    }
    FlagExpr& operator=(FlagExpr&& other) {
        left = std::move(other.left);
        right = std::move(other.right);
        op = other.op;
        name = std::move(other.name);
        value = other.value;
        return *this;
    }

    static FlagExpr Or(FlagExpr&& left, FlagExpr&& right) {
        return FlagExpr{ std::move(left), std::move(right), FlagExprType::Or };
    }
    static FlagExpr And(FlagExpr&& left, FlagExpr&& right) {
        return FlagExpr{ std::move(left), std::move(right), FlagExprType::And };
    }
    static FlagExpr Eq(FlagExpr&& left, FlagExpr&& right) {
        return FlagExpr{ std::move(left), std::move(right), FlagExprType::Eq };
    }
    static FlagExpr Ne(FlagExpr&& left, FlagExpr&& right) {
        return FlagExpr{ std::move(left), std::move(right), FlagExprType::Ne };
    }
    static FlagExpr Lt(FlagExpr&& left, FlagExpr&& right) {
        return FlagExpr{ std::move(left), std::move(right), FlagExprType::Lt };
    }
    static FlagExpr Gt(FlagExpr&& left, FlagExpr&& right) {
        return FlagExpr{ std::move(left), std::move(right), FlagExprType::Gt };
    }
    static FlagExpr Le(FlagExpr&& left, FlagExpr&& right) {
        return FlagExpr{ std::move(left), std::move(right), FlagExprType::Le };
    }
    static FlagExpr Ge(FlagExpr&& left, FlagExpr&& right) {
        return FlagExpr{ std::move(left), std::move(right), FlagExprType::Ge };
    }
    static FlagExpr Not(FlagExpr&& val) {
        return FlagExpr{ std::move(val), FlagExprType::Not };
    }
    static FlagExpr Value(uint32_t val) {
        return FlagExpr{ val, FlagExprType::Value };
    }
    static FlagExpr Identifier(std::string&& val) {
        return FlagExpr{ std::move(val), FlagExprType::Identifier };
    }

    static FlagExpr Default() {
        return FlagExpr{ "", FlagExprType::Default };
    }
    static FlagExpr Once() {
        return FlagExpr{ "", FlagExprType::Once };
    }
    static FlagExpr Random(std::string&& id, uint32_t mod) {
        return FlagExpr{ std::move(id), mod, FlagExprType::Random };
    }

    static FlagExpr True()  { return FlagExpr::Value(1); }
    static FlagExpr False() { return FlagExpr::Value(0); }

    int evaluate() const;
};


FlagExpr flagexpr_from_string(const std::string& expr);
bool operator==(const FlagExpr& a, const FlagExpr& b);
std::ostream& operator<<(std::ostream& stream, const FlagExpr& value);

