#include "pch.h"
#include "tracker.h"
#include "time/deltatime.h"


float Euclidean(const sf::Vector2i& a, const sf::Vector2i& b) { const auto d = sf::Vector2f(b-a); return std::sqrt(d.x * d.x + d.y * d.y); }
float Chebyshev(const sf::Vector2i& a, const sf::Vector2i& b) { const auto d = sf::Vector2f(b-a); return std::max(std::abs(d.x), std::abs(d.y)); }
const auto H = Chebyshev;


#define ROOT_2 1.414f


void PathTracker::trim_path_radial() {
    if (path_trim == 0.f || path.empty()) { return; }
    const auto last = path.back();
    for (int i = (int)path.size() - 2; i >= 0; i--) {
        const auto dist = sf::Vector2f(path[i] - last).lengthSquared();
        if (dist < path_trim * path_trim) {
            path.pop_back();
        } else {
            break;
        }
    }
    path_trim = 0.f;
}
void PathTracker::trim_path_walked() {
    if (path_trim == 0.f || path.empty()) { return; }
    auto total = 0.f;
    for (int i = (int)path.size() - 2; i >= 0; i--) {
        total += sf::Vector2f(path[i] - path[i+1]).length();
        if (total < path_trim) {
            path.pop_back();
        } else {
            break;
        }
    }
    path_trim = 0.f;
}
void PathTracker::clamp_path_radial() {
    if (path_clamp == std::numeric_limits<float>::infinity() || path.empty()) { return; }
    for (int i = (int)path.size() - 1; i >= 0; i--) {
        const auto dist = sf::Vector2f(path.front() - path[i]).lengthSquared();
        if (dist > path_clamp * path_clamp) {
            path.pop_back();
        } else {
            break;
        }
    }
    path_clamp = std::numeric_limits<float>::infinity();
}
void PathTracker::clamp_path_walked() {
    if (path_clamp == std::numeric_limits<float>::infinity() || path.empty()) { return; }
    auto total = 0.f;
    auto count = 0;
    for (size_t i = 1; i < path.size(); i++) {
        total += sf::Vector2f(path[i+1] - path[i]).length();
        if (total <= path_clamp) {
            count++;
        } else {
            break;
        }
    }
    path.resize(count);
    path_clamp = std::numeric_limits<float>::infinity();
}



bool PathTracker::set_position_grid(const sf::Vector2i& pos) {
    if (!grid->has_vertex(pos)) { return false; }
    position = into_worldspace(pos);
    path.clear();
    path_index = 0;
    path_prog = 0;
    return true;
}
bool PathTracker::set_position_world(const sf::Vector2f& pos) {
    if (!grid->has_vertex(into_gridspace(pos))) { return false; }
    position = pos;
    path.clear();
    path_index = 0;
    path_prog = 0;
    return true;
}

sf::Vector2i PathTracker::get_position_grid() const {
    if (is_moving()) {
        return path[path_index];
    } else {
        return into_gridspace(get_position_world());
    }
}
sf::Vector2f PathTracker::get_position_world() const {
    return position;
}

sf::Vector2i PathTracker::get_target_position_grid() const {
    if (is_moving()) {
        return path.back();
    } else {
        return into_gridspace(get_position_world());
    }
}
sf::Vector2f PathTracker::get_target_position_world() const {
    return into_worldspace(get_target_position_grid());
}

bool PathTracker::set_path_grid(const sf::Vector2i& goal) {
    if (grid->has_vertex(goal)) {
        path_queue = goal;
        return true;
    }
    return false;
}
bool PathTracker::set_path_world(const sf::Vector2f& goal) {
    return set_path_grid(into_gridspace(goal));
}

void PathTracker::trim_path_radial_grid(float dist) {
    path_trim = dist;
}
void PathTracker::trim_path_radial_world(float dist) {
    path_trim = dist / scale;
}

void PathTracker::trim_path_walked_grid(float dist) {
    std::cout << "unimplemented, performing radial\n";
    path_trim = dist;
}
void PathTracker::trim_path_walked_world(float dist) {
    std::cout << "unimplemented, performing radial\n";
    path_trim = dist / scale;
}

void PathTracker::clamp_path_radial_grid(float max) {
    path_clamp = max;
}
void PathTracker::clamp_path_radial_world(float max) {
    path_clamp = max / scale;
}

void PathTracker::clamp_path_walked_grid(float max) {
    std::cout << "unimplemented, performing radial\n";
    path_clamp = max;
}
void PathTracker::clamp_path_walked_world(float max) {
    std::cout << "unimplemented, performing radial\n";
    path_clamp = max / scale;
}

void PathTracker::progress() {
    if (override_stop) { return; }

    if (!is_moving()) {
        if (path_queue.has_value()) {
            path = grid->a_star(into_gridspace(position), *path_queue, H);
            path_index = 0;
            path_prog = 0;
            path_queue = {};
            trim_path_radial();
            clamp_path_radial();
        } else if (!path.empty()) {
            position = into_worldspace(path.back());
        }
        return;
    }

    if (path_index < path.size() - 1) {
        if (grid->get_vertex(path[path_index + 1]) >= MAX_PENALTY) {
            path.clear();
            return;
        }
        auto diff = path[path_index + 1] - path[path_index];
        auto diag = std::abs(diff.x) > 0 && std::abs(diff.y) > 0;
        auto thresh = diag ? ROOT_2 : 1.f;
        path_prog += speed * Time::deltatime() * 60.f;
        auto didit = false;

        while (path_prog >= thresh && path_index < path.size() - 1) {
            path_prog -= thresh;
            path_index++;
            if (path_index < path.size() - 1) {
                if (grid->get_vertex(path[path_index + 1]) >= MAX_PENALTY) {
                    path.clear();
                    return;
                }
                diff = path[path_index + 1] - path[path_index];
                diag = std::abs(diff.x) > 0 && std::abs(diff.y) > 0;
                thresh = diag ? ROOT_2 : 1.f;
            }
            didit = true;
        }

        if (path_queue.has_value() && didit) {
            const auto temp = path[path_index];
            path = grid->a_star(temp, *path_queue, H);
            position = into_worldspace(temp);
            path_index = 0;
            path_prog = 0;
            path_queue = {};
            trim_path_radial();
            clamp_path_radial();
        }

        if (path_index < path.size() - 1) {
            const auto pre = into_worldspace(path[path_index]);
            const auto post = into_worldspace(path[path_index + 1]);
            const auto pre_grid = path[path_index];
            const auto post_grid = path[path_index + 1];
            const auto dir = post - pre;
            const auto dir_grid = post_grid - pre_grid;
            const auto diag_correct = (std::abs(dir_grid.x) && std::abs(dir_grid.y)) ? ROOT_2 : 1.f;
            position = pre + dir * (path_prog / diag_correct);
        }
    } else if (path_queue.has_value()) {
        path = grid->a_star(into_gridspace(position), *path_queue, H);
        path_index = 0;
        path_prog = 0;
        path_queue = {};
        trim_path_radial();
        clamp_path_radial();
    }
}

