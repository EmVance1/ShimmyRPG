#include "pch.h"
#include "str.h"
#include "stralloc.h"


namespace fstr {


String str::to_string(Allocator<char>& alloc) const { return String::copy_from(*this, alloc); }


std::optional<char> str::get(size_t n) const {
    if (n < m_len) {
        return m_begin[n];
    } else {
        return {};
    }
}

char str::operator[](size_t n) const {
    if (n < m_len) {
        return m_begin[n];
    } else {
        throw std::invalid_argument("string index out of bounds");
    }
}


std::optional<size_t> str::find(const str pat) const {
    for (size_t i = 0; i < (len() - pat.len()); i++) {
        if (str{ data() + i, pat.len() } == pat) {
            return i;
        }
    }

    return {};
}

String str::replace(const str pat, const str to, Allocator<char>& alloc) const {
    String res = to_string(alloc);

    while (const auto idx = res.find(pat)) {
        auto temp = String::with_capacity(len() - pat.len() + to.len(), alloc);
        for (size_t i = 0; i < *idx; i++) {
            temp.push(res.data()[i]);
        }
        for (size_t i = 0; i < to.len(); i++) {
            temp.push(to.data()[i]);
        }
        for (size_t i = 0; i < (res.len() - pat.len() - *idx); i++) {
            temp.push(res.data()[*idx + pat.len() + i]);
        }
        res = std::move(temp);
    }

    return res;
}

String str::replacen(const str pat, const str to, size_t n, Allocator<char>& alloc) const {
    size_t m = 0;
    String res = to_string(alloc);

    while (const auto idx = res.find(pat)) {
        if (m++ == n) { break; }

        auto temp = String::with_capacity(len() - pat.len() + to.len(), alloc);
        for (size_t i = 0; i < *idx; i++) {
            temp.push(res.data()[i]);
        }
        for (size_t i = 0; i < to.len(); i++) {
            temp.push(to.data()[i]);
        }
        for (size_t i = 0; i < (res.len() - pat.len() - *idx); i++) {
            temp.push(res.data()[*idx + pat.len() + i]);
        }
        res = std::move(temp);
    }

    return res;
}


std::ostream& operator<<(std::ostream& stream, str val) {
    return stream.write(val.data(), val.len());
}
bool operator==(str self, str rhs) {
    if (self.len() != rhs.len()) {
        return false;
    }
    for (size_t i = 0; i < self.len(); i++) {
        if (self[i] != rhs[i]) {
            return false;
        }
    }
    return true;
}
bool operator!=(str a, str b) {
    return !(a == b);
}


template<> String to_string(const str& self, Allocator<char>& alloc) {
    return self.to_string(alloc);
}

}

