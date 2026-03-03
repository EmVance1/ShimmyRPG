#pragma once
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <string>
#include <cstdint>


struct Item {
    enum class Rarity { Unique = -1, Common = 0, Notable, Rare, Prized };
    std::string name;
    uint32_t stack;
    uint32_t weight;
    uint32_t value;
    Rarity rarity;
    std::unordered_set<std::string> tags;
};

class ItemDB {
private:
    static std::vector<Item> registry;
    static std::unordered_map<std::string, uint32_t> lookup;

public:
    static uint32_t create_item(const std::string& name, const Item& item);
    static uint32_t get_item_id(const std::string& name);
    static Item& get_item(uint32_t id);
    static Item& get_item_by_name(const std::string& name);
};

