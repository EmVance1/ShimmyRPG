#include <random>



class Random {
private:
    static std::random_device seeder;
    static std::mt19937 gen;

public:
    static size_t reseed();

    static int64_t integer(int64_t min, int64_t max);

    static uint32_t d(uint32_t sides);
    static uint32_t d4();
    static uint32_t d6();
    static uint32_t d8();
    static uint32_t d10();
    static uint32_t d12();
    static uint32_t d20();
    static uint32_t d100();
};

