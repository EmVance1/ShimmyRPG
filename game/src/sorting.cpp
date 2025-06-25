#include "pch.h"
#include "sorting.h"


std::vector<Entity*> sprites_topo_sort(std::unordered_map<std::string, Entity>& entities) {
    auto result = std::vector<Entity*>();
    result.reserve(entities.size());

    auto overlap_graph_out = std::unordered_map<std::string, std::unordered_set<std::string>>();
    auto overlap_graph_in  = std::unordered_map<std::string, std::unordered_set<std::string>>();
    auto S = std::unordered_set<std::string>();

    for (const auto& [k, v] : entities) {
        for (const auto& [l, w] : entities) {
            if (k != l && v.get_AABB().findIntersection(w.get_AABB()).has_value() && v.get_sorting_boundary() < w.get_sorting_boundary()) {
                overlap_graph_out[k].insert(l);
                overlap_graph_in[l].insert(k);
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

    return result;
}

Entity* top_contains(const std::vector<Entity*>& sorted_entities, const sf::Vector2f& point) {
    Entity* top = nullptr;
    for (Entity* e : sorted_entities) {
        if (e->contains(point)) {
            top = e;
        }
    }
    return top;
}

