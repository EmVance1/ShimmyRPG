#pragma once
#include <SFML/Graphics.hpp>
#include <unordered_map>
#include <vector>
#include "graph.h"


namespace std {
    template<typename T>
    class hash<sf::Vector2<T>> {
    public:
        std::uint64_t operator()(const sf::Vector2<T>& v) const {
            return (53 + std::hash<T>()(v.y)) * 53 + std::hash<T>()(v.x);
        }
    };
}


class SpatialGraph2d : public Graph<sf::Vector2i, float> {
public:
    struct Vertex {
        float weight;
        std::vector<sf::Vector2i> edges;
        std::vector<float> edge_weights;
    };

protected:
    std::unordered_map<sf::Vector2i, Vertex> m_graph;
    sf::Vector2i m_size;
    bool m_is_grid;

public:
    SpatialGraph2d() : m_size(0, 0), m_is_grid(false) {}
    SpatialGraph2d(const sf::Vector2u& size) : m_size(size), m_is_grid(true) {}
    SpatialGraph2d(const SpatialGraph2d& other) : m_graph(other.m_graph), m_size(other.m_size), m_is_grid(other.m_is_grid) {}

    SpatialGraph2d& operator=(const SpatialGraph2d& other) {
        m_graph = other.m_graph;
        m_size = other.m_size;
        m_is_grid = other.m_is_grid;
        return *this;
    }

    static SpatialGraph2d create_grid(const sf::Vector2u& size);

    void remove_vertex(const sf::Vector2i& id);
    void set_vertex_weight(const sf::Vector2i& id, float weight);

    bool has_vertex(const sf::Vector2i& id) const override { return m_graph.contains(id); }
    const float& get_vertex(const sf::Vector2i& id) const override { return m_graph.at(id).weight; }
    float& get_vertex(const sf::Vector2i& id) override { return m_graph.at(id).weight; }
    const std::vector<sf::Vector2i>& get_edges(const sf::Vector2i& id) const override { return m_graph.at(id).edges; }
    std::vector<sf::Vector2i>& get_edges(const sf::Vector2i& id) override { return m_graph.at(id).edges; }
    const std::vector<float>& get_edge_weights(const sf::Vector2i& id) const override { return m_graph.at(id).edge_weights; }
    std::vector<float>& get_edge_weights(const sf::Vector2i& id) override { return m_graph.at(id).edge_weights; }

    bool is_directed() const override { return true; }
};


SpatialGraph2d load_grid_from_image(const sf::Image& img);

