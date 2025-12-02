#include "pch.h"
#include "core/str.h"
#include "core/stralloc.h"
#include <iostream>


namespace shmy { namespace core {

String str::to_string() const {
    return String::from(*this);
}


Option<usize> str::find(const str pat) const {
    for (usize i = 0; i < (len() - pat.len()); i++) {
        if (str{ data() + i, pat.len() } == pat) {
            return Option<usize>::Some(std::move(i));
        }
    }

    return Option<usize>::None();
}

String str::replace(const str pat, const str to) const {
    String res = to_string();

    while (const auto idx = res.find(pat).if_let()) {
        auto temp = String::with_capacity(len() - pat.len() + to.len());
        for (usize i = 0; i < *idx; i++) {
            temp.push(res.data()[i]);
        }
        for (usize i = 0; i < to.len(); i++) {
            temp.push(to.data()[i]);
        }
        for (usize i = 0; i < (res.len() - pat.len() - *idx); i++) {
            temp.push(res.data()[*idx + pat.len() + i]);
        }
        res = temp.move();
    }

    return res;
}

String str::replacen(const str pat, const str to, usize n) const {
    usize m = 0;
    String res = to_string();

    while (const auto idx = res.find(pat).if_let()) {
        if (m == n) {
            break;
        }
        m++;

        auto temp = String::with_capacity(len() - pat.len() + to.len());
        for (usize i = 0; i < *idx; i++) {
            temp.push(res.data()[i]);
        }
        for (usize i = 0; i < to.len(); i++) {
            temp.push(to.data()[i]);
        }
        for (usize i = 0; i < (res.len() - pat.len() - *idx); i++) {
            temp.push(res.data()[*idx + pat.len() + i]);
        }
        res = temp.move();
    }

    return res;
}


std::ostream& operator<<(std::ostream& stream, str self) {
    return stream.write(self.data(), (std::streamsize)self.len());
}

bool operator==(str self, str rhs) {
    if (self.len() != rhs.len()) {
        return false;
    }
    for (usize i = 0; i < self.len(); i++) {
        if (self.nth(i) != rhs.nth(i)) {
            return false;
        }
    }
    return true;
}
bool operator!=(str self, str rhs) {
    return !(self == rhs);
}


} }

