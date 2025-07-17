#pragma once
#include <SFML/Graphics.hpp>
#include <navmesh/types.h>


namespace nav {

sf::Vector2f into_sf(const nav::Vector2f& v);
nav::Vector2f from_sf(const sf::Vector2f& v);

}


class PathTracker {
public:
    constexpr static float MAX_PENALTY = 100000.f;

private:
    const nav::NavMesh* mesh = nullptr;
    nav::Path path;
    size_t path_index = 0;
    float path_prog = 0.f;

    sf::Vector2f position;
    float speed = 1.0f;

    bool override_stop = false;

public:
    PathTracker(const nav::NavMesh* _mesh) : mesh(_mesh) {}

    void set_speed(float _speed) { speed = _speed; }
    float get_speed() const { return speed; }

    bool set_position(const sf::Vector2f& pos);
    const sf::Vector2f& get_position() const;

    bool set_target_position(const sf::Vector2f& goal);
    sf::Vector2f get_target_position() const;

    void trim_path_radial(float dist);
    void trim_path_walked(float dist);

    void clamp_path_radial(float max);
    void clamp_path_walked(float max);

    const nav::Path& get_active_path() const { return path; }
    float get_active_path_length() const;

    size_t get_current_index() const { return path_index; }
    size_t get_inverse_index() const { return path.size() - 1 - path_index; }

    bool is_moving() const { return !(path.empty() || (path_index == path.size() - 1) || override_stop); }
    void pause() { override_stop = true; }
    void stop()  { override_stop = true; path.clear(); path_index = 0; path_prog = 0; }
    void start() { override_stop = false; }

    void progress();
};

