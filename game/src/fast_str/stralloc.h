#pragma once
#include "str.h"
#include <optional>
#include <string>


namespace fstr {

class String {
private:
    char* m_data;
    size_t m_cap = 0;
    size_t m_size = 0;
    Allocator<char>* p_alloc;

private:
    String(const String& other);
    String(const String& other, Allocator<char>& alloc);
    String& operator=(const String& other);

public:
    String(Allocator<char>& alloc = Malloc<char>::Inst);
    String(String&& other);
    ~String();

    String& operator=(String&& other);

    static String make(Allocator<char>& alloc = Malloc<char>::Inst) { return String(alloc); }
    static String with_capacity(size_t cap, Allocator<char>& alloc = Malloc<char>::Inst);
    static String repeat(char c, size_t n, Allocator<char>& alloc = Malloc<char>::Inst);
    static String copy_from(str val, Allocator<char>& alloc = Malloc<char>::Inst);
    static String copy_from(const char* val, Allocator<char>& alloc = Malloc<char>::Inst);
    static String copy_from(const std::string& val, Allocator<char>& alloc = Malloc<char>::Inst);
    String clone(Allocator<char>& alloc = Malloc<char>::Inst) const { return String(*this, alloc); }

    char* begin() { return data(); }
    const char* begin() const { return data(); }
    const char* cbegin() const { return data(); }
    char* end() { return data() + m_size; }
    const char* end() const { return data() + m_size; }
    const char* cend() const { return data() + m_size; }

    void reserve(size_t cap);
    void push(char val);
    std::optional<char> pop();
    void insert(size_t index, char val);
    std::optional<char> remove(size_t index);
    void clear();

    std::optional<char> get(size_t n) const;
    const char& operator[](size_t n) const;
    char& operator[](size_t n);

    std::optional<size_t> find(const str pat) const;
    String replace(const str pat, const str to) const;
    String replacen(const str pat, const str to, size_t n) const;

    bool is_empty() const { return m_size == 0; }
    size_t len() const { return m_size; }
    size_t capacity() const { return m_cap; }

    const char* data() const { return m_data; }
    char* data() { return m_data; }
    const char* c_str() const { return data(); }
    str as_str() const { return str(data(), m_size); }

    const str operator*() const { return as_str(); }
    const str operator->() const { return as_str(); }
};

template<typename T>
fstr::String to_string(const T&, Allocator<char>& alloc = Malloc<char>::Inst) {
    static_assert(false, "'ToString' is not implemented for this type");
    return fstr::String::make(alloc);
}

}

std::ostream& operator<<(std::ostream& stream, const fstr::String& val);
bool operator==(const fstr::String& a, const fstr::String& b);
bool operator!=(const fstr::String& a, const fstr::String& b);

namespace fstr {

template<> String to_string(const str& self,      Allocator<char>& alloc);
template<> String to_string(const String& self,   Allocator<char>& alloc);
template<> String to_string(const int8_t& self,   Allocator<char>& alloc);
template<> String to_string(const int16_t& self,  Allocator<char>& alloc);
template<> String to_string(const int32_t& self,  Allocator<char>& alloc);
template<> String to_string(const int64_t& self,  Allocator<char>& alloc);
template<> String to_string(const uint8_t& self,  Allocator<char>& alloc);
template<> String to_string(const uint16_t& self, Allocator<char>& alloc);
template<> String to_string(const uint32_t& self, Allocator<char>& alloc);
template<> String to_string(const uint64_t& self, Allocator<char>& alloc);
template<> String to_string(const size_t& self,   Allocator<char>& alloc);
template<> String to_string(const size_t& self,   Allocator<char>& alloc);
template<> String to_string(const float& self,    Allocator<char>& alloc);
template<> String to_string(const double& self,   Allocator<char>& alloc);

}

