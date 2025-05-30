#include "pch.h"
#include "str.h"
#include "str_util.h"


namespace fstr {

fstr::Split str::split(char pat) const {
    return fstr::Split(*this, pat);
}


Split::Split(str val, char pat) : m_val(val), m_pat(pat) {
}

std::optional<str> Split::next() {
    return {};
}


}


