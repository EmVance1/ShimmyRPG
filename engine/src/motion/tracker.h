#pragma once
#include "algo/graph2d.h"


class PathTracker {
private:
    const SpatialGraph2d* grid = nullptr;
    SpatialGraph2d::Path path;
    size_t path_index = 0;
    float path_prog = 0.f;
    std::optional<sf::Vector2i> path_queue;

    sf::Vector2f position;
    float speed = 0.1f;
    float scale = 1.f;

    bool override_stop = false;

public:
    constexpr static float MAX_PENALTY = 100000.f;

    PathTracker(const SpatialGraph2d* _grid, float _scale) : grid(_grid), scale(_scale) {}

    sf::Vector2f into_worldspace(const sf::Vector2i& v) const { return sf::Vector2f(v) * scale; }
    sf::Vector2i into_gridspace(const sf::Vector2f& v) const { return sf::Vector2i(v * (1.f / scale)); }

    void set_speed(float _speed) { speed = _speed * 0.1f; }
    float get_speed() const { return speed * 10.f; }

    bool set_position_grid(const sf::Vector2i& pos);
    bool set_position_world(const sf::Vector2f& pos);

    sf::Vector2i get_position_grid() const;
    sf::Vector2f get_position_world() const;

    sf::Vector2i get_target_position_grid() const;
    sf::Vector2f get_target_position_world() const;

    bool set_path_grid(const sf::Vector2i& goal);
    bool set_path_world(const sf::Vector2f& goal);

    const SpatialGraph2d::Path& get_active_path() const { return path; }
    size_t get_current_index() const { return path_index; }
    size_t get_inverse_index() const { return path.size() - path_index - 1; }

    bool is_moving() const { return !path.empty() && !(path_index == path.size() - 1) && !override_stop; }
    void stop() { override_stop = true; }
    void start() { override_stop = false; }

    void progress();

};

