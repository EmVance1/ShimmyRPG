#pragma once
#include <SFML/Graphics.hpp>
#include <unordered_map>
#include <unordered_set>
#include <limits>
#include <vector>
#include <queue>
#include <concepts>


template<typename T>
struct DijkstraTuple {
    T id;
    T parent;
    float cost = std::numeric_limits<float>::infinity();
};

template<typename T>
auto operator<=>(const DijkstraTuple<T>& a, const DijkstraTuple<T>& b) { return a.cost <=> b.cost; }
template<typename T>
auto operator==(const DijkstraTuple<T>& a, const DijkstraTuple<T>& b) { return a.id == b.id && a.parent == b.parent && a.cost == b.cost; }

template<typename T>
struct AStarTuple {
    T id;
    T parent;
    float g_cost = std::numeric_limits<float>::infinity();
    float f_cost = std::numeric_limits<float>::infinity();
};

template<typename T>
auto operator<=>(const AStarTuple<T>& a, const AStarTuple<T>& b) { return a.f_cost <=> b.f_cost; }
template<typename T>
auto operator==(const AStarTuple<T>& a, const AStarTuple<T>& b) { return a.id == b.id && a.parent == b.parent && a.g_cost == b.g_cost && a.f_cost == b.f_cost; }

template<typename T>
using LowPrioQueue = std::priority_queue<T, std::vector<T>, std::greater<T>>;
template<typename T>
using HighPrioQueue = std::priority_queue<T, std::vector<T>, std::less<T>>;


template<typename T>
concept Hashable = requires(T a)
{
    { std::hash<T>{}(a) } -> std::convertible_to<std::size_t>;
};

template<Hashable K, typename V>
class Graph {
public:
    using Path = std::vector<K>;

    virtual bool has_vertex(const K& id) const = 0;
    virtual const V& get_vertex(const K& id) const = 0;
    virtual V& get_vertex(const K& id) = 0;
    virtual const std::vector<K>& get_edges(const K& id) const = 0;
    virtual std::vector<K>& get_edges(const K& id) = 0;
    virtual const std::vector<float>& get_edge_weights(const K& id) const = 0;
    virtual std::vector<float>& get_edge_weights(const K& id) = 0;
    virtual bool is_directed() const = 0;

    std::vector<K> dijkstra(const K& begin, const K& end) const {
        auto queue = MinHeap<DijkstraTuple<K>>();
        queue.push(DijkstraTuple{ begin, begin, 0 });

        auto lut = std::unordered_map<K, DijkstraTuple<K>>();
        lut[begin] = DijkstraTuple{ begin, begin, 0 };

        while (!queue.is_empty()) {
            const auto current = queue.pop();

            const auto c_cost = lut[current.id].cost;
            for (size_t i = 0; i < get_edges(current.id).size(); i++) {
                const auto n_id = get_edges(current.id)[i];
                const auto n_cost = c_cost + get_edge_weights(current.id)[i];
                if (n_cost < lut[n_id].cost) {
                    lut[n_id] = DijkstraTuple{
                        n_id,
                        current.id,
                        n_cost,
                    };
                    if (!queue.contains(lut[n_id])) {
                        queue.push(lut[n_id]);
                    }
                }
            }
        }

        if (lut.contains(end)) {
            auto current = end;
            Path total_path = { end };
            while (lut.contains(current) && (lut.at(current).parent != current)) {
                current = lut.at(current).parent;
                total_path.insert(total_path.begin(), current);
            }
            total_path.pop_back();
            return total_path;
        } else {
            return {};
        }
    }

    std::vector<K> a_star(const K& begin, const K& end, float(*h)(const K&, const K&)) const {
        auto queue = LowPrioQueue<AStarTuple<K>>();
        queue.push(AStarTuple{ begin, begin, 0, h(begin, end) });

        auto lut = std::unordered_map<K, AStarTuple<K>>();
        lut[begin] = AStarTuple{ begin, begin, 0, h(begin, end) };

        while (!queue.empty()) {
            const auto current = queue.top();
            queue.pop();

            if (current.id == end) {
                if (lut.contains(end)) {
                    auto cur = end;
                    Path total_path = { end };
                    while (lut.contains(cur) && (lut.at(cur).parent != cur)) {
                        cur = lut.at(cur).parent;
                        total_path.insert(total_path.begin(), cur);
                    }
                    return total_path;
                } else {
                    return {};
                }
            }

            const auto c_g_cost = lut[current.id].g_cost;
            for (size_t i = 0; i < get_edges(current.id).size(); i++) {
                const auto n_id = get_edges(current.id)[i];
                const auto g_cost_tentative = c_g_cost + get_edge_weights(current.id)[i];
                if (!lut.contains(n_id)) {
                    lut[n_id] = AStarTuple{
                        n_id,
                        n_id,
                        std::numeric_limits<float>::infinity(),
                        std::numeric_limits<float>::infinity(),
                    };
                }
                const auto g_cost_neighbor = lut[n_id].g_cost;
                if (g_cost_tentative < g_cost_neighbor) {
                    lut[n_id] = AStarTuple{
                        n_id,
                        current.id,
                        g_cost_tentative,
                        g_cost_tentative + h(n_id, end),
                    };
                    // if (!queue.contains(lut[n_id])) {
                        queue.push(lut[n_id]);
                    // }
                }
            }
        }

        return {};
    }
};


template<typename K, typename V>
class AdjacencyGraph : public Graph<K, V> {
public:
    struct Vertex {
        V data;
        std::vector<K> edges;
        std::vector<K> edge_weights;
    };

private:
    std::unordered_map<K, Vertex> m_graph;
    bool m_directed;

public:
    AdjacencyGraph(bool directed) : m_directed(directed) {}

    bool has_vertex(const K& id) const override {
        return m_graph.contains(id);
    }
    const V& get_vertex(const K& id) const {
        return m_graph.at(id).data;
    }
    V& get_vertex(const K& id) {
        return m_graph.at(id).data;
    }
    const std::vector<K>& get_edges(const K& id) const {
        return m_graph.at(id).edges;
    }
    std::vector<K>& get_edges(const K& id) {
        return m_graph.at(id).edges;
    }
    const std::vector<float>& get_edge_weights(const K& id) const {
        return m_graph.at(id).edge_weights;
    }
    std::vector<float>& get_edge_weights(const K& id) {
        return m_graph.at(id).edge_weights;
    }

    bool is_directed() const override { return m_directed; }
};

