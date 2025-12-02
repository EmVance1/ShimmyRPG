#pragma once
#include "num.h"
#include "str.h"
#include "option.h"
#include <string>


namespace shmy { namespace core {

class String {
private:
    char* m_data = nullptr;
    size_t m_cap = 0;
    size_t m_size = 0;

private:
    String() = default;
    String(const String& other);
    String& operator=(const String& other);

public:
    String(String&& other);
    ~String();
    String& operator=(String&& other);

    static String make();
    static String with_capacity(usize cap);
    static String repeat(char c, usize n);
    static String from(str val);
    static String from(const std::string& val);
    String&& move() { return std::move(*this); }
    String clone() const { return String(*this); }

    // typename std::vector<T>::iterator begin() { return impl.begin(); }
    // typename std::vector<T>::const_iterator begin() const { return impl.cbegin(); }
    // typename std::vector<T>::const_iterator cbegin() const { return impl.cbegin(); }
    // typename std::vector<T>::iterator end() { return impl.end(); }
    // typename std::vector<T>::const_iterator end() const { return impl.cend(); }
    // typename std::vector<T>::const_iterator cend() const { return impl.cend(); }

    void reserve(usize cap);
    void push(char val);
    Option<char> pop();
    void insert(usize index, char val);
    Option<char> remove(usize index);
    void clear();

    Option<char> nth(usize n) const;

    Option<usize> find(const str pat) const;
    String replace(const str pat, const str to) const;
    String replacen(const str pat, const str to, usize n) const;

    bool is_empty() const { return m_size == 0; }
    usize len() const { return m_size; }
    usize capacity() const { return m_cap; }

    const char* data() const { return m_data; }
    char* data() { return m_data; }
    const char* c_str() const { return m_data; }
    str as_str() const { return str(m_data, m_size); }

    str operator*() { return as_str(); }
    const str operator*() const { return as_str(); }

    str operator->() { return as_str(); }
    const str operator->() const { return as_str(); }

    template<typename T>
    friend String to_string(const T&);
};

template<typename T>
shmy::core::String to_string(const T&) {
    static_assert(false, "'ToString' is not implemented for this type");
    return shmy::core::String::make();
}

} }


std::ostream& operator<<(std::ostream& stream, const shmy::core::String& val);
std::istream& operator>>(std::istream& stream, shmy::core::String& val);
bool operator==(const shmy::core::String& a, const shmy::core::String& b);
bool operator!=(const shmy::core::String& a, const shmy::core::String& b);

namespace shmy { namespace core {

template<> shmy::core::String to_string(const shmy::core::String& self);
template<> shmy::core::String to_string(const i8& self);
template<> shmy::core::String to_string(const i16& self);
template<> shmy::core::String to_string(const i32& self);
template<> shmy::core::String to_string(const i64& self);
template<> shmy::core::String to_string(const u8& self);
template<> shmy::core::String to_string(const u16& self);
template<> shmy::core::String to_string(const u32& self);
template<> shmy::core::String to_string(const u64& self);
template<> shmy::core::String to_string(const usize& self);
template<> shmy::core::String to_string(const f32& self);
template<> shmy::core::String to_string(const f64& self);

} }


