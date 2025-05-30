#pragma once
#include <memory>
#include <string>
#include <vector>


class Content {
public:
    virtual std::string into() const = 0;
    virtual bool read_change() const = 0;
    virtual bool consume_change() = 0;
};


template<typename T>
class SetSignal {
private:
    std::shared_ptr<T> data;
    std::shared_ptr<size_t> signal;

public:
    SetSignal(std::shared_ptr<T> _data, std::shared_ptr<size_t> _signal) : data(_data), signal(_signal) {}

    void set(const T& value) { *data = value; (*signal)++; }
};

template<typename T>
class GetSignal : public Content {
private:
    std::shared_ptr<T> data;
    std::shared_ptr<size_t> signal;

public:
    GetSignal(std::shared_ptr<T> _data, std::shared_ptr<size_t> _signal) : data(_data), signal(_signal) {}

    const T& get() { return *data; }
    std::string into() const override { return std::to_string(*data); }
    bool read_change() const override { if (*signal > 0) { return true; } return false; }
    bool consume_change() override { if (*signal > 0) { (*signal)--; return true; } return false; }
};

class StringWrapper : public Content {
private:
    std::string data;

public:
    StringWrapper(const std::string& value) : data(value) {}

    std::string into() const override { return data; }
    bool read_change() const override { return false; }
    bool consume_change() override { return false; }
};

template<typename T>
struct Signal {
    GetSignal<T> get;
    SetSignal<T> set;
};

template<typename T>
Signal<T> new_signal(const T& val) {
    auto shared = std::make_shared<T>(val);
    auto signal = std::make_shared<size_t>(0);
    return Signal<T>{ GetSignal<T>( shared, signal ), SetSignal<T>( shared, signal ) };
}


template<typename T>
std::unique_ptr<Content> into_content(const GetSignal<T>& value) {
    return std::make_unique<GetSignal<T>>(value);
}
std::unique_ptr<Content> into_content(const std::string& value);


class DynamicContent {
private:
    std::vector<std::unique_ptr<Content>> m_list;
    std::string m_cache;

private:
    void set_dynamic_impl() {}
    template<typename T>
    void set_dynamic_impl(const T& val) {
        m_list.emplace_back(into_content(val));
    }
    template<typename T, typename ... Args>
    void set_dynamic_impl(const T& val, Args&& ... args) {
        m_list.emplace_back(into_content(val));
        set_dynamic_impl(std::forward<Args>(args)...);
    }

    bool consume_changes() {
        bool result = false;
        for (auto& elem : m_list) {
            if (elem->consume_change()) {
                result = true;
            }
        }
        return result;
    }
    std::string concatenate() const {
        std::string result = "";
        for (const auto& elem : m_list) {
            result += elem->into();
        }
        return result;
    }

public:
    void set_static(const std::string& value) {
        m_list.push_back(std::make_unique<StringWrapper>(value));
    }

    template<typename ... Args>
    void set_dynamic(Args&& ... args) {
        set_dynamic_impl(std::forward<Args>(args)...);
    }

    const std::string& get() {
        if (consume_changes()) {
            m_cache = concatenate();
        }
        return m_cache;
    }
};

