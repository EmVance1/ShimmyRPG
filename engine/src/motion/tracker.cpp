#include "pch.h"
#include "tracker.h"
#include "time/deltatime.h"


namespace nav {

sf::Vector2f into_sf(const nav::Vector2f& v) { return sf::Vector2f(v.x, v.y); }
nav::Vector2f from_sf(const sf::Vector2f& v) { return nav::Vector2f(v.x, v.y); }

}


bool PathTracker::set_position(const sf::Vector2f& pos) {
    if (!mesh->get_triangle(nav::from_sf(pos), 0.05f).has_value()) { return false; }
    position = pos;
    path.clear();
    path_index = 0;
    path_prog = 0;
    return true;
}

const sf::Vector2f& PathTracker::get_position() const {
    return position;
}


bool PathTracker::set_target_position(const sf::Vector2f& goal) {
    path = mesh->pathfind(nav::from_sf(position), nav::from_sf(goal));
    if (path.empty()) { return false; }
    path_index = 0;
    path_prog = 0;
    return true;
}

sf::Vector2f PathTracker::get_target_position() const {
    if (path.empty()) {
        return position;
    } else {
        return nav::into_sf(path.back());
    }
}


static std::optional<float> ray_circle_intersect_nearest(nav::Vector2f p, nav::Vector2f d, nav::FloatCircle c) {
    const float x = c.pos.dot(c.pos) + p.dot(p) - 2 * c.pos.dot(p) - c.radius * c.radius;
    const float y = 2 * d.dot(c.pos - p);
    const float z = d.dot(d);
    const float disc = y * y - 4 * x * z;
    if (disc < 0) { return {}; }
    const float root = std::sqrt(disc);
    const float t1 = (y + root) / (2 * z);
    if (std::abs(disc) < 0.0001f) {
        return t1 < 0 ? std::optional<float>{} : t1;
    }
    const float t2 = (y - root) / (2 * z);
    if (t1 < 0 && t2 < 0) { return {}; }
    if (t1 < 0) { return t2; }
    if (t2 < 0) { return t1; }
    return t1 < t2 ? t1 : t2;
}

void PathTracker::trim_path_radial(float dist) {
    if (dist == 0.f || path.empty()) { return; }
    const auto last = path.back();

    for (size_t j = 0; j < path.size(); j++) {
        const auto i = path.size() - 1;
        const auto d1 = nav::Vector2f(path[i] - last).length_squared();

        if (d1 <= dist * dist) {
            const auto d2 = nav::Vector2f(path[i-1] - last).length_squared();
            if (d2 <= dist * dist) {
                path.pop_back();
            } else {
                const auto pos = path[i-1];
                const auto dir = (path[i] - path[i-1]).normalise();
                const auto circle = nav::FloatCircle{ last, dist };
                path[i] = pos + dir * ray_circle_intersect_nearest(pos, dir, circle).value();
                return;
            }
        }
    }
}

void PathTracker::trim_path_walked(float dist) {
    if (dist == 0.f || path.empty()) { return; }
    auto total = 0.f;
    for (int i = (int)path.size() - 2; i >= 0; i--) {
        total += nav::Vector2f(path[i] - path[i+1]).length();
        if (total < dist) {
            path.pop_back();
        } else {
            break;
        }
    }
}

void PathTracker::clamp_path_radial(float dist) {
    if (path.empty()) { return; }

    std::cout << "BROKEN; FIX OR DONT USE\n";
    exit(1);

    const auto first = path.front();

    for (size_t j = 0; j < path.size(); j++) {
        const auto i = path.size() - 1;
        const auto d1 = nav::Vector2f(path[i] - first).length_squared();

        if (d1 >= dist * dist) {
            const auto d2 = nav::Vector2f(path[i-1] - first).length_squared();
            if (d2 >= dist * dist) {
                path.pop_back();
            } else {
                const auto pos = path[i-1];
                const auto dir = (path[i] - path[i-1]).normalise();
                const auto circle = nav::FloatCircle{ first, dist };
                path[i] = pos + dir * ray_circle_intersect_nearest(pos, dir, circle).value();
                return;
            }
        }
    }
}

void PathTracker::clamp_path_walked(float dist) {
    if (path.empty()) { return; }
    auto total = 0.f;
    auto count = 1ULL;
    auto step = 0.f;
    for (size_t i = 0; i < path.size()-1 && total < dist; i++) {
        step = nav::Vector2f(path[i+1] - path[i]).length();
        total += step;
        count++;
    }
    if (total <= dist) { return; }
    if (count < 2) { path.clear(); return; }
    path.resize(count);
    const auto pos = path[path.size() - 2];
    const auto dir = (path[path.size() - 1] - pos).normalise();
    path.back() = pos + dir * (step - (total - dist));
}


float PathTracker::get_active_path_length() const {
    float result = 0.f;
    for (size_t i = 0; i < path.size() - 1; i++) {
        result += (path[i] - path[i+1]).length();
    }
    return result;
}


void PathTracker::progress() {
    if (!is_moving()) { return; }

    if (nav::into_sf(path[path_index + 1]) == position) {
        if (++path_index == path.size() - 1) { return; }
    }
    const auto diff = nav::into_sf(path[path_index + 1]) - position;
    const auto dist = diff.length();
    if (dist < speed) {
        path_index++;
        position = nav::into_sf(path[path_index]);
        const auto dir = diff * (1.f / dist);
        position += dir * (speed - dist);
    } else {
        const auto dir = diff * (1.f / dist);
        position += dir * speed;
    }
}
