#include "pch.h"
#include "item.h"


std::vector<Item> ItemDB::registry;
std::unordered_map<std::string, uint32_t> ItemDB::lookup;


uint32_t ItemDB::create_item(const std::string& name, const Item& item) {
    const auto id = (uint32_t)registry.size();
    lookup[name] = id;
    registry.push_back(item);
    return id;
}

uint32_t ItemDB::get_item_id(const std::string& name) {
    return lookup.at(name);
}

Item& ItemDB::get_item(uint32_t id) {
    return registry[(size_t)id];
}

Item& ItemDB::get_item_by_name(const std::string& name) {
    return get_item(lookup.at(name));
}
