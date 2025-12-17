#include "pch.h"
#include "sorting.h"


std::vector<Entity*> sprites_topo_sort(std::unordered_map<std::string, Entity>& entities) {
    auto overlap_graph_out = std::unordered_map<std::string, std::unordered_set<std::string>>();
    auto overlap_graph_in  = std::unordered_map<std::string, std::unordered_set<std::string>>();
    auto S = std::unordered_set<std::string>();
    auto result = std::vector<Entity*>();

    /*
    for (const auto& [k, e] : entities) {
        for (const auto& [l, f] : entities) {
            if (k != l && e.get_AABB().findIntersection(f.get_AABB()).has_value() && e.get_boundary() < f.get_boundary()) {
                overlap_graph_out[k].insert(l);
                overlap_graph_in [l].insert(k);
            }
        }
    }
    for (const auto& [k, _] : entities) {
        if (overlap_graph_in[k].empty()) {
            S.insert(k);
        }
    }
    while (!S.empty()) {
        const auto n = *S.begin();
        S.erase(n);
        result.push_back(&entities.at(n));
        for (const auto& e : overlap_graph_out[n]) {
            overlap_graph_in[e].erase(n);
            if (overlap_graph_in[e].empty()) {
                S.insert(e);
            }
        }
    }
    */

    return result;
}

/*
Entity* top_contains(const std::vector<Entity*>& sorted_entities, const sf::Vector2f& point) {
    Entity* top = nullptr;
    for (Entity* e : sorted_entities) {
        if (e->contains(point)) {
            top = e;
        }
    }
    return top;
}
*/

