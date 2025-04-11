#include "pch.h"
#include "bench.h"
#include <chrono>


sf::Time benchmark_function(std::function<void()> f) {
    const auto t1 = std::chrono::high_resolution_clock::now();
    f();
    const auto t2 = std::chrono::high_resolution_clock::now();
    return sf::microseconds(std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count());
}

