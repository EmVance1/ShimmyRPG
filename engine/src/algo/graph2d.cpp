#include "pch.h"
#include "graph2d.h"
#include "motion/tracker.h"


SpatialGraph2d SpatialGraph2d::create_grid(const sf::Vector2u& size) {
    auto result = SpatialGraph2d(size);
    for (int y = 0; y < (int)size.y; y++) {
        for (int x = 0; x < (int)size.x; x++) {
            result.m_graph[sf::Vector2i(x, y)] = Vertex();
            auto& ref = result.m_graph[sf::Vector2i(x, y)];
            ref.edges.reserve(8);
            ref.edge_weights.reserve(8);
            for (int i = -1; i < 2; i++) {
                for (int j = -1; j < 2; j++) {
                    if (!(i == 0 && j == 0) && y + i >= 0 && y + i < (int)size.y && x + j >= 0 && x + j < (int)size.x) {
                        ref.edges.push_back({x + j, y + i});
                        ref.edge_weights.push_back((i != 0 && j != 0) ? 1.414f : 1.f);
                    }
                }
            }
        }
    }
    return result;
}

void SpatialGraph2d::remove_vertex(const sf::Vector2i& id) {
    if (!m_graph.contains(id)) { return; }
    for (const auto& e : m_graph[id].edges) {
        auto& ref = m_graph[e];
        const auto it = std::find(ref.edges.begin(), ref.edges.end(), id);
        const size_t idx = it - ref.edges.begin();
        ref.edges.erase(ref.edges.begin() + idx);
        ref.edge_weights.erase(ref.edge_weights.begin() + idx);
    }
    m_graph.erase(id);
}

void SpatialGraph2d::set_vertex_weight(const sf::Vector2i& id, float weight) {
    if (!m_graph.contains(id)) { return; }
    for (const auto& e : m_graph[id].edges) {
        auto& ref = m_graph[e];
        const auto it = std::find(ref.edges.begin(), ref.edges.end(), id);
        const auto idx = it - ref.edges.begin();
        const auto dif = e - id;
        ref.edge_weights[idx] = weight * ((dif.x != 0 && dif.y != 0) ? 1.414f : 1.f);
    }
    m_graph[id].weight = weight;
}


SpatialGraph2d load_grid_from_image(const sf::Image& img) {
    auto pathfind = SpatialGraph2d::create_grid(img.getSize());

    for (uint32_t y = 0; y < img.getSize().y; y++) {
        for (uint32_t x = 0; x < img.getSize().x; x++) {
            if (img.getPixel({x, y}).r == 255) {
                pathfind.set_vertex_weight({(int)x, (int)y}, PathTracker::MAX_PENALTY);
            } else {
                pathfind.set_vertex_weight({(int)x, (int)y}, 4.f * ((float)img.getPixel({x, y}).r / 255.f) + 1.f);
            }
        }
    }

    return pathfind;
}

