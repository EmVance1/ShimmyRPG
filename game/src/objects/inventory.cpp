#include "pch.h"
#include "inventory.h"
#include "item.h"


uint32_t Inventory::total_weight() const {
    uint32_t n = 0;
    for (const auto& i : store) {
        n += ItemDB::get_item(i.item_id).weight * i.count;
    }
    return n;
}

