#pragma once
#include "num.h"
#include <iosfwd>


namespace shmy { namespace core {

template<typename T>
class Option;
class String;
class Split;


class str {
private:
    const char* m_begin;
    usize m_len;

public:
    constexpr str(const char* begin, usize len) : m_begin(begin), m_len(len) {}
    constexpr str(const str& other) : m_begin(other.m_begin), m_len(other.m_len) {}
    str& operator=(const str& other) { m_begin = other.m_begin; m_len = other.m_len; return *this; }

    String to_string() const;

    constexpr const char* data() const { return m_begin; }
    constexpr usize len() const { return m_len; }

    constexpr char nth(usize n) const { return m_begin[n]; }
    constexpr str slice(usize begin, usize end) const { return str{ m_begin + begin, end - begin }; }

    Option<usize> find(const str pat) const;
    String replace(const str pat, const str to) const;
    String replacen(const str pat, const str to, usize n) const;

    Split split(char pat) const;
};

#define STR(val) ::shmy::core::str{ val, sizeof(val) - 1 }

std::ostream& operator<<(std::ostream& stream, str val);
bool operator==(str lhs, str rhs);
bool operator!=(str lhs, str rhs);


} }
