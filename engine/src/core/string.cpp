#include "pch.h"
#include "core/stralloc.h"
#include <cstring>
#include <string>


namespace shmy { namespace core {


String::String(const String& other)
    : m_data((char*)calloc(other.m_cap+1, sizeof(char))), m_cap(other.m_cap), m_size(other.m_size)
{
    memcpy(m_data, other.m_data, m_size * sizeof(char));
}
String& String::operator=(const String& other) {
    m_data = (char*)calloc(other.m_cap+1, sizeof(char));
    m_cap = other.m_cap;
    m_size = other.m_size;
    memcpy(m_data, other.m_data, m_cap * sizeof(char));
    return *this;
}

String::String(String&& other) noexcept
    : m_data(other.m_data), m_cap(other.m_cap), m_size(other.m_size)
{
    other.m_data = nullptr;
}
String::~String() {
    free(m_data);
}

String& String::operator=(String&& other) noexcept {
    m_data = other.m_data;
    m_cap = other.m_cap;
    m_size = other.m_size;
    other.m_data = nullptr;
    return *this;
}

String String::make() {
    auto result = String();
    result.m_data = (char*)calloc(5, sizeof(char));
    result.m_cap = 4;
    result.m_size = 0;
    return result;
}
String String::with_capacity(usize cap) {
    auto result = String();
    result.m_data = (char*)calloc(cap+1, sizeof(char));
    result.m_cap = cap;
    result.m_size = 0;
    return result;
}
String String::repeat(char c, usize n) {
    auto result = String();
    result.m_data = (char*)calloc(n+1, sizeof(char));
    result.m_cap = n;
    result.m_size = n;
    memset(result.m_data, (int)c, n);
    return result;
}
String String::from(str val) {
    auto result = String();
    result.m_data = (char*)calloc(val.len()+1, sizeof(char));
    result.m_cap = val.len();
    result.m_size = val.len();
    memcpy(result.m_data, val.data(), val.len() * sizeof(char));
    return result;
}
String String::from(const std::string& val) {
    auto result = String();
    result.m_data = (char*)calloc(val.size()+1, sizeof(char));
    result.m_cap = val.size();
    result.m_size = val.size();
    memcpy(result.m_data, val.data(), val.size() * sizeof(char));
    return result;
}

// typename std::vector<T>::iterator begin() { return impl.begin(); }
// typename std::vector<T>::const_iterator begin() const { return impl.cbegin(); }
// typename std::vector<T>::const_iterator cbegin() const { return impl.cbegin(); }
// typename std::vector<T>::iterator end() { return impl.end(); }
// typename std::vector<T>::const_iterator end() const { return impl.cend(); }
// typename std::vector<T>::const_iterator cend() const { return impl.cend(); }

void String::reserve(usize cap) {
    char* temp = (char*)calloc(cap+1, sizeof(char));
    if (cap > m_cap) {
        memcpy(temp, m_data, m_size * sizeof(char));
    } else {
        memcpy(temp, m_data, cap * sizeof(char));
    }
    free(m_data);
    m_data = temp;
    m_cap = cap;
}

void String::push(char val) {
    m_data[m_size] = val;
    if (++m_size == m_cap) {
        reserve(m_cap * 2);
    }
}

Option<char> String::pop() {
    if (is_empty()) {
        return Option<char>::None();
    } else {
        char temp = m_data[--m_size];
        m_data[m_size] = 0;
        return Option<char>::Some(std::move(temp));
    }
}

void String::insert(usize index, char val) {
    for (size_t i = m_size; i > (size_t)index; i--) {
        m_data[i] = m_data[i - 1];
    }
    m_data[(size_t)index] = val;
    m_size += 1;
    if (m_size == m_cap) {
        reserve(m_cap * 2);
    }
}

Option<char> String::remove(usize index) {
    if (is_empty()) {
        return Option<char>::None();
    } else {
        char temp = m_data[(size_t)index];
        for (size_t i = (size_t)index; i < --m_size; i++) {
            m_data[i] = m_data[i + 1];
        }
        m_data[m_size] = 0;
        return Option<char>::Some(std::move(temp));
    }
}

void String::clear() {
    free(m_data);
    m_data = (char*)calloc(5, sizeof(char));
    m_cap = 4;
    m_size = 0;
}


Option<char> String::nth(usize n) const {
    if (n < m_size) {
        return Option<char>::Some(std::move(m_data[n]));
    } else {
        return Option<char>::None();
    }
}


Option<usize> String::find(const str pat) const {
    for (usize i = 0; i < (len() - pat.len() + 1); i++) {
        if (str{ data() + i, pat.len() } == pat) {
            return Option<usize>::Some(std::move(i));
        }
    }

    return Option<usize>::None();
}

String String::replace(const str pat, const str to) const {
    String res = clone();

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

String String::replacen(const str pat, const str to, usize n) const {
    usize m = 0;
    String res = clone();

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

} }


std::ostream& operator<<(std::ostream& stream, const shmy::core::String& val) {
    return stream.write(val.data(), (std::streamsize)val.len());
}
std::istream& operator>>(std::istream& stream, shmy::core::String& val) {
    auto temp = std::string("");
    stream >> temp;
    val = shmy::core::String::from(temp);
    return stream;
}
bool operator==(const shmy::core::String& a, const shmy::core::String& b) {
    if (a.len() != b.len()) return false;
    return strncmp(a.data(), b.data(), a.len()) == 0;
}
bool operator!=(const shmy::core::String& a, const shmy::core::String& b) {
    return !(a == b);
}

namespace shmy { namespace core {

template<> shmy::core::String to_string(const shmy::core::String& self) {
    return self.clone();
}
template<> shmy::core::String to_string(const i8& self) {
    return to_string((i64)self);
}
template<> shmy::core::String to_string(const i16& self) {
    return to_string((i64)self);
}
template<> shmy::core::String to_string(const i32& self) {
    return to_string((i64)self);
}
template<> shmy::core::String to_string(const i64& self) {
    i64 acc = self;
    char buf[32];
    int buflen = 0;
    while (acc != 0) {
        buf[buflen++] = '0' + (acc % 10);
        acc /= 10;
    }
    if (self < 0) {
        buf[buflen++] = '-';
    }
    auto result = String::with_capacity((usize)buflen);
    for (int i = 0; i < buflen; i++) {
        result.m_data[buflen - i - 1] = buf[i];
    }
    return result;
}
template<> shmy::core::String to_string(const u8& self) {
    return to_string((u64)self);
}
template<> shmy::core::String to_string(const u16& self) {
    return to_string((u64)self);
}
template<> shmy::core::String to_string(const u32& self) {
    return to_string((u64)self);
}
template<> shmy::core::String to_string(const u64& self) {
    u64 acc = self;
    char buf[32];
    int buflen = 0;
    while (acc > 0) {
        buf[buflen++] = '0' + (acc % 10);
        acc /= 10;
    }
    auto result = String::with_capacity((usize)buflen);
    for (int i = 0; i < buflen; i++) {
        result.m_data[buflen - i - 1] = buf[i];
    }
    return result;
}
template<> shmy::core::String to_string(const f32& self) {
    return String::from(std::to_string(self));
}
template<> shmy::core::String to_string(const f64& self) {
    return String::from(std::to_string(self));
}

} }

