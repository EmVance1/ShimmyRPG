#pragma once
#include <optional>
#include <iostream>
#include "alloc.h"


namespace fstr {

class String;
class Split;

class str {
private:
    const char* const m_begin;
    const size_t m_len;

public:
    constexpr str(const char* const begin, size_t len) : m_begin(begin), m_len(len) {}

    fstr::String to_string(Allocator<char>& alloc = Malloc<char>::Inst) const;

    constexpr const char* data() const { return m_begin; }
    constexpr size_t len() const { return m_len; }

    constexpr char at(size_t n) const { return m_begin[n]; }
    constexpr str slice(size_t begin, size_t end) const { return str{ m_begin + begin, end - begin }; }

    std::optional<char> get(size_t n) const;
    char operator[](size_t n) const;

    std::optional<size_t> find(const str pat) const;
    fstr::String replace(const str pat, const str to, Allocator<char>& alloc = Malloc<char>::Inst) const;
    fstr::String replacen(const str pat, const str to, size_t n, Allocator<char>& alloc = Malloc<char>::Inst) const;

    fstr::Split split(char pat) const;
};


#define STR(val) str{ val, sizeof(val) - 1 }

std::ostream& operator<<(std::ostream& stream, str val);
bool operator==(str lhs, str rhs);
bool operator!=(str lhs, str rhs);

}

