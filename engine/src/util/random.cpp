#include "pch.h"
#include "util/random.h"



std::random_device Random::seeder;
std::mt19937 Random::gen{ Random::seeder() };


size_t Random::reseed() {
    const auto seed = seeder();
    gen.seed(seed);
    return seed;
}


int64_t Random::integer(int64_t min, int64_t max) {
    auto dist = std::uniform_int_distribution<int64_t>(min, max);
    return dist(gen);
}

uint32_t Random::d(uint32_t sides) {
    auto dist = std::uniform_int_distribution<uint32_t>(1, sides);
    return dist(gen);
}

uint32_t Random::d4() {
    return (uint32_t)d(4);
}
uint32_t Random::d6() {
    return (uint32_t)d(6);
}
uint32_t Random::d8() {
    return (uint32_t)d(8);
}
uint32_t Random::d10() {
    return (uint32_t)d(10);
}
uint32_t Random::d12() {
    return (uint32_t)d(12);
}
uint32_t Random::d20() {
    return (uint32_t)d(20);
}
uint32_t Random::d100() {
    return (uint32_t)d(100);
}

