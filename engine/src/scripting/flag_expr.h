#pragma once
#include <string>
#include <memory>


namespace shmy {

struct FlagExpr {
    enum class Type {
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

    std::unique_ptr<FlagExpr> left;
    std::unique_ptr<FlagExpr> right;
    FlagExpr::Type op;
    std::string name;
    uint32_t value;

    FlagExpr()
        : left(nullptr), right(nullptr), op(FlagExpr::Type::Value), name(""), value(0)
    {}
    FlagExpr(const FlagExpr& other)
        : left(other.left ? std::make_unique<FlagExpr>(*other.left) : nullptr),
        right(other.right ? std::make_unique<FlagExpr>(*other.right) : nullptr),
        op(other.op), name(other.name), value(other.value)
    {}
    FlagExpr(FlagExpr&& other)
        : left(std::move(other.left)), right(std::move(other.right)), op(other.op), name(std::move(other.name)), value(other.value)
    {}
    FlagExpr(FlagExpr&& _left, FlagExpr&& _right, FlagExpr::Type _op)
        : left(std::make_unique<FlagExpr>(std::move(_left))), right(std::make_unique<FlagExpr>(std::move(_right))), op(_op)
    {}
    FlagExpr(FlagExpr&& _val, FlagExpr::Type _op)
        : left(std::make_unique<FlagExpr>(std::move(_val))), right(nullptr), op(_op)
    {}
    FlagExpr(std::string&& _val, FlagExpr::Type _op)
        : left(nullptr), right(nullptr), op(_op), name(std::move(_val)), value(0)
    {}
    FlagExpr(uint32_t _val, FlagExpr::Type _op)
        : left(nullptr), right(nullptr), op(_op), name(""), value(_val)
    {}
    FlagExpr(std::string&& _id, uint32_t _mod, FlagExpr::Type _op)
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
        return FlagExpr{ std::move(left), std::move(right), FlagExpr::Type::Or };
    }
    static FlagExpr And(FlagExpr&& left, FlagExpr&& right) {
        return FlagExpr{ std::move(left), std::move(right), FlagExpr::Type::And };
    }
    static FlagExpr Eq(FlagExpr&& left, FlagExpr&& right) {
        return FlagExpr{ std::move(left), std::move(right), FlagExpr::Type::Eq };
    }
    static FlagExpr Ne(FlagExpr&& left, FlagExpr&& right) {
        return FlagExpr{ std::move(left), std::move(right), FlagExpr::Type::Ne };
    }
    static FlagExpr Lt(FlagExpr&& left, FlagExpr&& right) {
        return FlagExpr{ std::move(left), std::move(right), FlagExpr::Type::Lt };
    }
    static FlagExpr Gt(FlagExpr&& left, FlagExpr&& right) {
        return FlagExpr{ std::move(left), std::move(right), FlagExpr::Type::Gt };
    }
    static FlagExpr Le(FlagExpr&& left, FlagExpr&& right) {
        return FlagExpr{ std::move(left), std::move(right), FlagExpr::Type::Le };
    }
    static FlagExpr Ge(FlagExpr&& left, FlagExpr&& right) {
        return FlagExpr{ std::move(left), std::move(right), FlagExpr::Type::Ge };
    }
    static FlagExpr Not(FlagExpr&& val) {
        return FlagExpr{ std::move(val), FlagExpr::Type::Not };
    }
    static FlagExpr Value(uint32_t val) {
        return FlagExpr{ val, FlagExpr::Type::Value };
    }
    static FlagExpr Identifier(std::string&& val) {
        return FlagExpr{ std::move(val), FlagExpr::Type::Identifier };
    }

    static FlagExpr Default() {
        return FlagExpr{ "", FlagExpr::Type::Default };
    }
    static FlagExpr Once() {
        return FlagExpr{ "", FlagExpr::Type::Once };
    }
    static FlagExpr Random(std::string&& id, uint32_t mod) {
        return FlagExpr{ std::move(id), mod, FlagExpr::Type::Random };
    }

    static FlagExpr True()  { return FlagExpr::Value(1); }
    static FlagExpr False() { return FlagExpr::Value(0); }

    int evaluate() const;

    static FlagExpr from_string(const std::string& expr);
};

bool operator==(const FlagExpr& a, const FlagExpr& b);
std::ostream& operator<<(std::ostream& stream, const FlagExpr& value);

}
