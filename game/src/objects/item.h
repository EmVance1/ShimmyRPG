#pragma once
#include <cstdint>
#include <string>
#include <unordered_set>


struct Item {
    uint32_t id;
    std::string name;
    std::unordered_set<std::string> properties;
};

struct ItemEntry {
    Item item;
    uint32_t count = 0;
};

