#pragma once
#include <string>
#include <deque>
#include <sstream>


namespace shmy::core {

class Logger {
    inline static std::deque<std::string> buffer;
    inline static size_t maxLines = 500;

public:
    template<typename... Args>
    static void info(Args&&... args) {
        std::ostringstream oss;
        oss << "[info] ";
        (oss << ... << args);
        append(oss.str());
    }
    template<typename... Args>
    static void warn(Args&&... args) {
        std::ostringstream oss;
        oss << "[warn] ";
        (oss << ... << args);
        append(oss.str());
    }
    template<typename... Args>
    static void error(Args&&... args) {
        std::ostringstream oss;
        oss << "[err]  ";
        (oss << ... << args);
        append(oss.str());
    }

    static std::string get_log() {
        std::ostringstream oss;
        for (const auto& line : buffer) {
            oss << line << "\n";
        }
        return oss.str();
    }

private:
    static void append(const std::string& msg) {
        if (buffer.size() >= maxLines) {
            buffer.pop_front();
        }
        buffer.push_back(msg);
    }
};

}

