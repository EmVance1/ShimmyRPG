#include "pch.h"
#include "world/scene.h"
#include "world/game.h"


void Scene::sprites_sort() {
    auto overlap_graph_out = std::unordered_map<size_t, std::unordered_set<uint32_t>>();
    auto overlap_graph_in  = std::unordered_map<size_t, std::unordered_set<uint32_t>>();
    auto S = std::unordered_set<uint32_t>();

    for (const auto& V : entities) {
        auto& v = game->entity(V);
        for (const auto& W : entities) {
            auto& w = game->entity(W);
            if (V != W && v.get_AABB().findIntersection(w.get_AABB()).has_value() && w.get_sorting_boundary() < v.get_sorting_boundary()) {
                overlap_graph_in[V].insert(W);
                overlap_graph_out[W].insert(V);
            }
        }
        if (overlap_graph_in[V].empty()) {
            S.insert(V);
        }
    }

    entities.clear();
    while (!S.empty()) {
        const auto n = *S.begin();
        S.erase(n);
        entities.push_back(n);
        for (const auto& e : overlap_graph_out[n]) {
            overlap_graph_in[e].erase(n);
            if (overlap_graph_in[e].empty()) {
                S.insert(e);
            }
        }
    }
}


uint32_t Scene::sprites_top(const sf::Vector2f& point) {
    for (auto it = entities.rbegin(); it != entities.rend(); it++) {
        if (game->entity(*it).contains(point)) {
            return *it;
        }
    }
    return Entity::INVALID;
}

