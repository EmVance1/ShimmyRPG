#include "pch.h"
#include "tracker.h"
#include "time/deltatime.h"


float Euclidean(const sf::Vector2i& a, const sf::Vector2i& b) { const auto d = sf::Vector2f(b-a); return std::sqrt(d.x * d.x + d.y * d.y); }
float Chebyshev(const sf::Vector2i& a, const sf::Vector2i& b) { const auto d = sf::Vector2f(b-a); return std::max(std::abs(d.x), std::abs(d.y)); }
const auto H = Chebyshev;


bool PathTracker::set_position_grid(const sf::Vector2i& pos) {
    if (!grid->has_vertex(pos)) {
        return false;
    }
    position = into_worldspace(pos);
    path.clear();
    path_index = 0;
    path_prog = 0;
    return true;
}
bool PathTracker::set_position_world(const sf::Vector2f& pos) {
    if (!grid->has_vertex(into_gridspace(pos))) {
        return false;
    }
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
    const auto g = into_gridspace(goal);
    if (grid->has_vertex(g)) {
        path_queue = g;
        return true;
    }
    return false;
}

void PathTracker::progress() {
    if (!is_moving()) {
        if (path_queue.has_value()) {
            path = grid->a_star(into_gridspace(position), *path_queue, H);
            path_index = 0;
            path_prog = 0;
            path_queue = {};
            if (path.empty()) {
                path.push_back(into_gridspace(position));
                return;
            }
        } else {
            if (!path.empty()) {
                position = into_worldspace(path.back());
            }
            return;
        }
    }

    if (path_index < path.size() - 1) {
        if (grid->get_vertex(path[path_index + 1]) >= MAX_PENALTY) {
            path.clear();
            return;
        }
        auto diff = path[path_index + 1] - path[path_index];
        auto diag = std::abs(diff.x) > 0 && std::abs(diff.y) > 0;
        auto thresh = diag ? 1.4f : 1.f;
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
                thresh = diag ? 1.4f : 1.f;
            }
            didit = true;
        }

        if (path_queue.has_value() && didit) {
            const auto temp = path[path_index];
            position = into_worldspace(path[path_index]);
            path = grid->a_star(path[path_index], *path_queue, H);
            path_index = 0;
            path_prog = 0;
            path_queue = {};
            if (path.empty()) {
                path.push_back(temp);
                return;
            }
        }

        if (path_index < path.size() - 1) {
            const auto pre = into_worldspace(path[path_index]);
            const auto post = into_worldspace(path[path_index + 1]);
            const auto pre_grid = path[path_index];
            const auto post_grid = path[path_index + 1];
            const auto dir = post - pre;
            const auto dir_grid = post_grid - pre_grid;
            const auto diag_correct = (std::abs(dir_grid.x) && std::abs(dir_grid.y)) ? 1.4f : 1.f;
            position = pre + dir * (path_prog / diag_correct);
        }
    } else if (path_queue.has_value()) {
        path = grid->a_star(into_gridspace(position), *path_queue, H);
        path_index = 0;
        path_prog = 0;
        path_queue = {};
        if (path.empty()) {
            path.push_back(into_gridspace(position));
            return;
        }
    }
}

