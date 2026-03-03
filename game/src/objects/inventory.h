#pragma once
#include <vector>


struct Inventory {
    struct Slot {
        uint32_t item_id;
        uint32_t count;
    };

    std::vector<Slot> store;
    uint32_t currency = 0;

    uint32_t total_weight() const;
};

