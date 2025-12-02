#include "pch.h"
#include "core/str.h"
#include "core/stralloc.h"
#include "core/split.h"


namespace shmy { namespace core {

#define SOME(x) Option<decltype(x)>::Some(std::move(x))
#define NONE(T) Option<T>::None()


Split str::split(char pat) const {
    return Split(*this, pat);
}


Split::Split(const str val, char pat) : m_val(val), m_pat(pat) {}
Split::Split(const String& val, char pat) : m_val(val.as_str()), m_pat(pat) {}
Split::Split(const std::string& val, char pat) : m_val(val.data(), val.size()), m_pat(pat) {}


Option<const str> Split::next() {
    for (size_t i = m_index; i < m_val.len(); i++) {
        if (m_val.nth(i) == m_pat) {
            const auto temp = m_val.slice(m_index, i - m_index);
            m_index = i + 1;
            return SOME(temp);
        }
    }

    if (m_index < m_val.len()) {
        const auto temp = m_val.slice(m_index, m_val.len());
        m_index = m_val.len();
        return SOME(temp);
    } else {
        return NONE(const str);
    }
}


std::vector<std::string> split(const std::string& val, char pat) {
    auto result = std::vector<std::string>();
    size_t last = 0;
    bool dont = false;

    for (size_t i = 0; i < val.size(); i++) {
        if (val[i] == pat) {
            result.push_back(val.substr(last, i - last));
            if (i + 1 < val.size()) {
                last = i + 1;
            } else {
                dont = true;
            }
        }
    }

    if (!dont) {
        result.push_back(val.substr(last, val.size() - last));
    }

    return result;
}

std::string trim(const std::string& str) {
    size_t begin = 0;
    while (begin < str.size() && std::isspace(str[begin])) {
        begin++;
    }
    size_t len = str.size();
    while (len > 0 && std::isspace(str[len - 1])) {
        len--;
    }
    if (begin == str.size() || len == 0) {
        return "";
    } else {
        return str.substr(begin, len - begin);
    }
}

} }

