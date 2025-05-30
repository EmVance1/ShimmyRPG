#include "pch.h"
#include "stralloc.h"
#include <cstring>
#include <stdexcept>
#include <string>


using usize = uint64_t;
using isize = int64_t;


namespace fstr {


String String::with_capacity(size_t cap, Allocator<char>& alloc) {
    auto temp = String(alloc);
    temp.reserve(cap);
    return temp;
}
String String::repeat(char c, size_t n, Allocator<char>& alloc) {
    auto self = with_capacity(n, alloc);
    memset(self.data(), c, n);
    self.m_size = n;
    return self;
}
String String::copy_from(str val, Allocator<char>& alloc) {
    auto self = with_capacity(val.len(), alloc);
    memcpy(self.data(), val.data(), val.len());
    self.m_size = val.len();
    return self;
}
String String::copy_from(const char* val, Allocator<char>& alloc) {
    return copy_from(str(val, strlen(val)), alloc);
}
String String::copy_from(const std::string& val, Allocator<char>& alloc) {
    auto self = with_capacity(val.size(), alloc);
    memcpy(self.data(), val.data(), val.size());
    self.m_size = val.size();
    return self;
}


String::String(const String& other)
    : m_data(other.p_alloc->alloc_arr(other.m_cap)), m_cap(other.m_cap), m_size(other.m_size), p_alloc(other.p_alloc)
{
    memcpy(m_data, other.m_data, m_cap * sizeof(char));
}
String::String(const String& other, Allocator<char>& alloc)
    : m_data(alloc.alloc_arr(other.m_cap)), m_cap(other.m_cap), m_size(other.m_size), p_alloc(&alloc)
{
    memcpy(m_data, other.m_data, m_cap * sizeof(char));
}
String& String::operator=(const String& other) {
    m_data = other.p_alloc->alloc_arr(other.m_cap);
    m_cap = other.m_cap;
    m_size = other.m_size;
    memcpy(m_data, other.m_data, m_cap * sizeof(char));
    return *this;
}

String::String(Allocator<char>& alloc)
    : m_data(alloc.alloc_arr(4)), m_cap(4), m_size(0), p_alloc(&alloc)
{}
String::String(String&& other)
    : m_data(other.m_data), m_cap(other.m_cap), m_size(other.m_size), p_alloc(other.p_alloc)
{
    other.m_data = nullptr;
}
String::~String() {
    p_alloc->dealloc(m_data);
}

String& String::operator=(String&& other) {
    m_data = other.m_data;
    m_cap = other.m_cap;
    m_size = other.m_size;
    p_alloc = other.p_alloc;
    other.m_data = nullptr;
    return *this;
}


void String::reserve(size_t cap) {
    char* temp = p_alloc->alloc_arr(cap);
    memcpy(temp, m_data, std::min(cap, m_size * sizeof(char)));
    p_alloc->dealloc(m_data);
    m_data = temp;
    m_cap = cap;
}

void String::push(char val) {
    m_data[m_size++] = val;
    if (m_size == m_cap) {
        reserve(m_cap * 2);
    }
}

std::optional<char> String::pop() {
    if (is_empty()) {
        return {};
    }
    m_size--;
    const char temp = m_data[m_size];
    m_data[m_size] = 0;
    return temp;
}

void String::insert(size_t index, char val) {
    for (isize i = m_size; i >= (isize)index; i--) {
        m_data[i] = m_data[i-1];
    }
    m_data[index] = val;
    m_size++;
    if (m_size == m_cap) {
        reserve(m_cap * 2);
    }
}

std::optional<char> String::remove(size_t index) {
    if (is_empty()) {
        return {};
    }
    m_size--;
    const char temp = m_data[index];
    for (size_t i = index; i < m_size; i++) {
        m_data[i] = m_data[i+1];
    }
    m_data[m_size] = 0;
    return temp;
}

void String::clear() {
    p_alloc->dealloc(m_data);
    m_data = p_alloc->alloc_arr(4);
    m_cap = 4;
    m_size = 0;
}


std::optional<char> String::get(size_t n) const {
    return as_str().get(n);
}

char& String::operator[](size_t n) {
    if (n < m_size) {
        return m_data[n];
    } else {
        throw std::invalid_argument("string index out of bounds");
    }
}

const char& String::operator[](size_t n) const {
    if (n < m_size) {
        return m_data[n];
    } else {
        throw std::invalid_argument("string index out of bounds");
    }
}


std::optional<size_t> String::find(const str pat) const {
    return as_str().find(pat);
}

String String::replace(const str pat, const str to) const {
    return as_str().replace(pat, to);
}

String String::replacen(const str pat, const str to, size_t n) const {
    return as_str().replacen(pat, to, n);
}

}


std::ostream& operator<<(std::ostream& stream, const fstr::String& val) {
    return stream.write(val.data(), val.len());
}
bool operator==(const fstr::String& self, const fstr::String& rhs) {
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
bool operator!=(const fstr::String& self, const fstr::String& rhs) {
    return !(self == rhs);
}

namespace fstr {

template<> String to_string(const String& self, Allocator<char>& alloc) {
    return self.clone(alloc);
}
template<> String to_string(const int8_t&   self, Allocator<char>& alloc) { return String::copy_from(std::to_string(self), alloc); }
template<> String to_string(const int16_t&  self, Allocator<char>& alloc) { return String::copy_from(std::to_string(self), alloc); }
template<> String to_string(const int32_t&  self, Allocator<char>& alloc) { return String::copy_from(std::to_string(self), alloc); }
template<> String to_string(const int64_t&  self, Allocator<char>& alloc) { return String::copy_from(std::to_string(self), alloc); }
template<> String to_string(const uint8_t&  self, Allocator<char>& alloc) { return String::copy_from(std::to_string(self), alloc); }
template<> String to_string(const uint16_t& self, Allocator<char>& alloc) { return String::copy_from(std::to_string(self), alloc); }
template<> String to_string(const uint32_t& self, Allocator<char>& alloc) { return String::copy_from(std::to_string(self), alloc); }
template<> String to_string(const uint64_t& self, Allocator<char>& alloc) { return String::copy_from(std::to_string(self), alloc); }
template<> String to_string(const float&    self, Allocator<char>& alloc) { return String::copy_from(std::to_string(self), alloc); }
template<> String to_string(const double&   self, Allocator<char>& alloc) { return String::copy_from(std::to_string(self), alloc); }

}

