#pragma once
#include <variant>
#include "num.h"


namespace shmy { namespace core {

template<typename T, typename E>
class Result;


#define panic(...) fprintf(stderr, __VA_ARGS__); exit(1)

template<typename T>
class Option {
private:
    enum { VSome, VNone, } tag;
    std::variant<T, Unit> data;

    Option(const T& val) : tag(Option::VSome), data(val) {}
    Option(T&& val) :      tag(Option::VSome), data(std::move(val)) {}
    Option() :             tag(Option::VNone), data(Unit{})  {}

public:
    using Some_t = T;

    static Option Some(const T& val) { return Option(val); }
    static Option Some(T&& val)      { return Option(std::move(val)); }
    static Option None()             { return Option(); }

    bool is_some() const { return tag == Option::VSome; }
    bool is_none() const { return tag == Option::VNone; }

    T&& unwrap()                     { if (is_some()) { return std::move(std::get<T>(data)); } else { panic("called 'Option::unwrap' on a 'None' value"); } }
    T&& unwrap_or(const T& fallback) { if (is_some()) { return std::move(std::get<T>(data)); } else { return fallback; } }
    T&& expect(const char* msg)      { if (is_some()) { return std::move(std::get<T>(data)); } else { panic(msg); } }

    template<typename E>
    Option<T> ok_or(E err) { if (is_some()) { return Result<T, E>::Ok(std::get<T>(data)); } else { return Result<T, E>::Err(err); } }

    template<typename U>
    Option<U> map(U(*func)(T)) { if (is_some()) { return Option<U>::Some(func(std::get<T>(data))); } else { return Option<U>::None(); } }
    template<typename U>
    Option<U> and_then(Option<U>(*func)(T)) { if (is_some()) { return func(std::get<T>(data)); } else { return Option<U>::None(); } }

    Option<const T*> as_ref() const { if (is_some()) { return Option<const T*>::Some(&std::get<T>(data)); } else { return Option<const T*>::None(); } }
    Option<T*> as_mut() { if (is_some()) { return Option<T*>::Some(&std::get<T>(data)); } else { return Option<T*>::None(); } }

    const T* if_let() const { return std::get_if<T>(&data); }
    T* if_let_mut() { return std::get_if<T>(&data); }
};

} }

template<typename T>
bool operator==(shmy::core::Option<T> self, shmy::core::Option<T> rhs) {
    if (self.is_none() && rhs.is_none()) {
        return true;
    }
    if (auto l = self.if_let()) {
        if (auto r = rhs.if_let()) {
            return (*l == *r);
        }
    }
    return false;
}
template<typename T>
bool operator!=(shmy::core::Option<T> self, shmy::core::Option<T> rhs) {
    return !(self == rhs);
}

