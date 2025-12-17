#include "pch.h"
#include "editor.h"
#include <algorithm>
#include <iostream>
#include "navmesh/gen.h"


static std::vector<sf::Vertex> get_display_mesh(const nav::Mesh& mesh) {
    auto va = std::vector<sf::Vertex>();
    va.reserve(mesh.triangles.size() * 3);
    for (const auto tri : mesh.triangles) {
        const auto b_color = sf::Color((uint8_t)(rand() % 50) + 20, (uint8_t)(rand() % 150) + 105, (uint8_t)(rand() % 150) + 105, 100);
        const auto w_color = sf::Color((uint8_t)(rand() % 55) + 200, (uint8_t)(rand() % 100) + 55, (uint8_t)(rand() % 100) + 55, 100);
        const auto color = tri.weight > 1.f ? w_color : b_color;
        va.push_back(sf::Vertex{ sf::Vector2f(mesh.vertices[tri.A].x, mesh.vertices[tri.A].y), color });
        va.push_back(sf::Vertex{ sf::Vector2f(mesh.vertices[tri.B].x, mesh.vertices[tri.B].y), color });
        va.push_back(sf::Vertex{ sf::Vector2f(mesh.vertices[tri.C].x, mesh.vertices[tri.C].y), color });
    }
    return va;
}

static bool segment_intersects(nav::Vector2f a, nav::Vector2f b, nav::Vector2f p) {
    return ((p.y < a.y) != (p.y < b.y)) &&
        (p.x < (a.x + ((p.y-a.y)/(b.y-a.y) * (b.x - a.x))));
}

static bool poly_contains(const Polygon& poly, nav::Vector2f p) {
    size_t count = 0;
    for (size_t i = 0; i < poly.size(); i++) {
        const auto j = (i+1) % poly.size();
        if (segment_intersects(poly[i], poly[j], p)) {
            count++;
        }
    }
    return count % 2 == 1;
}

void NavmeshEditor::init_gizmos() {
    constexpr sf::Color color = sf::Color::Cyan;

    for (const auto& p : m_shapes.polys) {
        auto& e = m_poly_editors.emplace_back();
        for (const auto& v : p.poly) {
            auto& c = e.anchors.emplace_back();
            c.setPosition({ v.x, v.y });
            c.setRadius(5);
            c.setOrigin({ 5, 5 });
            c.setFillColor(color);
            e.display.push_back(sf::Vertex{ sf::Vector2f{ v.x, v.y }, color });
        }
        e.display.push_back(e.display.front());
    }
    for (const auto& c : m_shapes.circles) {
        auto& e = m_circle_editors.emplace_back();
        e.display.setPosition({ c.circle.pos.x, c.circle.pos.y });
        e.display.setRadius(c.circle.radius);
        e.display.setOrigin({ c.circle.radius, c.circle.radius });
        e.display.setFillColor(sf::Color::Transparent);
        e.display.setOutlineThickness(0.3f);
        e.display.setOutlineColor(color);

        e.anchor_pos.setPosition({ c.circle.pos.x, c.circle.pos.y });
        e.anchor_pos.setSize({ 10, 10 });
        e.anchor_pos.setOrigin({ 5, 5 });
        e.anchor_pos.setFillColor(color);

        e.anchor_size.setPosition({ c.circle.pos.x + c.circle.radius, c.circle.pos.y });
        e.anchor_size.setSize({ 10, 10 });
        e.anchor_size.setOrigin({ 5, 5 });
        e.anchor_size.setFillColor(color);
    }
}



void NavmeshEditor::compute_cutouts() {
    auto cutouts = std::vector<size_t>();
    for (const auto& c : m_shapes.circles) {
        if (c.is_cutout) {
            for (size_t i = 0; i < m_mesh.triangles.size(); i++) {
                const auto err = nav::FloatCircle{ c.circle.pos, c.circle.radius * 1.01f };
                if ((err.contains(m_mesh.vertices[m_mesh.triangles[i].A])) &&
                    (err.contains(m_mesh.vertices[m_mesh.triangles[i].B])) &&
                    (err.contains(m_mesh.vertices[m_mesh.triangles[i].C]))) {
                    cutouts.push_back(i);
                }
            }
        } else if (c.weight > 1.f) {
            for (size_t i = 0; i < m_mesh.triangles.size(); i++) {
                const auto err = nav::FloatCircle{ c.circle.pos, c.circle.radius * 1.01f };
                if ((err.contains(m_mesh.vertices[m_mesh.triangles[i].A])) &&
                    (err.contains(m_mesh.vertices[m_mesh.triangles[i].B])) &&
                    (err.contains(m_mesh.vertices[m_mesh.triangles[i].C]))) {
                    m_mesh.triangles[i].weight = c.weight;
                }
            }
        }
    }
    std::sort(cutouts.begin(), cutouts.end(), std::greater<>());
    for (size_t c : cutouts) {
        m_mesh.triangles.erase(m_mesh.triangles.begin() + c);
        m_mesh.edges.erase(m_mesh.edges.begin() + c);
        for (auto& edges : m_mesh.edges) {
            edges.erase(std::remove_if(edges.begin(), edges.end(), [=](const nav::Mesh::Edge& e){ return e.index == c; }), edges.end());
            for (auto& e2 : edges) {
                if (e2.index > c) {
                    e2.index--;
                }
            }
        }
    }

    cutouts.clear();
    for (const auto& p : m_shapes.polys) {
        if (p.is_cutout) {
            for (size_t i = 0; i < m_mesh.triangles.size(); i++) {
                if (poly_contains(p.poly, m_mesh.triangles[i].centroid(m_mesh.vertices.data()))) {
                    cutouts.push_back(i);
                }
            }
        } else if (p.weight > 1.f) {
            for (size_t i = 0; i < m_mesh.triangles.size(); i++) {
                if (poly_contains(p.poly, m_mesh.triangles[i].centroid(m_mesh.vertices.data()))) {
                    m_mesh.triangles[i].weight = p.weight;
                }
            }
        }
    }
    std::sort(cutouts.begin(), cutouts.end(), std::greater<>());
    for (size_t c : cutouts) {
        m_mesh.triangles.erase(m_mesh.triangles.begin() + c);
        m_mesh.edges.erase(m_mesh.edges.begin() + c);
        for (auto& edges : m_mesh.edges) {
            edges.erase(std::remove_if(edges.begin(), edges.end(), [=](const nav::Mesh::Edge& e){ return e.index == c; }), edges.end());
            for (auto& e2 : edges) {
                if (e2.index > c) {
                    e2.index--;
                }
            }
        }
    }
}

void NavmeshEditor::compute_mesh() {
    auto polys = std::vector<Polygon>();
    for (const auto& p : m_shapes.polys) {
        polys.push_back(p.poly);
    }

    auto circles = std::vector<nav::FloatCircle>();
    for (const auto& c : m_shapes.circles) {
        circles.push_back(c.circle);
    }

    try {
        m_mesh = nav::generate_from_shapes(polys, circles, {}, 8, 0.f);
        compute_cutouts();
        m_display = get_display_mesh(m_mesh);
    } catch (const std::exception& e) {
        std::cerr << e.what() << "\n";
        exit(1);
    }
}


bool NavmeshEditor::load_file(const std::filesystem::path& path) {
    if (const auto s = MeshFile::read_file(path)) {
        m_shapes = *s;
        compute_mesh();
        m_circle_editors.clear();
        m_poly_editors.clear();
        init_gizmos();
        return true;
    }
    return false;
}

bool NavmeshEditor::save_file(const std::filesystem::path& path) {
    if (m_shapes.write_file(path)) {
        std::cout << "saved: " << path.string() << "\n";
        return true;
    } else {
        std::cout << "failed to save: " << path.string() << "\n";
        return false;
    }
}

void NavmeshEditor::export_file(const std::filesystem::path& path) {
    m_mesh.write_file(path, true);
    std::cout << "exported: " << path.string() << "\n";
}


void NavmeshEditor::handle_event(const sf::Event& event, const sf::RenderWindow& window) {
    if (const auto mbp = event.getIf<sf::Event::MouseButtonPressed>()) {
        if (mbp->button != sf::Mouse::Button::Left) { return; }
        const auto mapped = m_transform.transformPoint(window.mapPixelToCoords(mbp->position));

        for (size_t i = 0; i < m_circle_editors.size(); i++) {
            if (m_circle_editors[i].anchor_pos.getGlobalBounds().contains(mapped)) {
                selected.type = SelectionType::Circle;
                selected.circle = i;
                selected.mode = SelectionMode::Position;
                return;
            } else if (m_circle_editors[i].anchor_size.getGlobalBounds().contains(mapped)) {
                selected.type = SelectionType::Circle;
                selected.circle = i;
                selected.mode = SelectionMode::Size;
                return;
            }
        }

        for (size_t i = 0; i < m_poly_editors.size(); i++) {
            for (size_t j = 0; j < m_poly_editors[i].anchors.size(); j++) {
                if (nav::FloatCircle{ m_shapes.polys[i].poly[j], 5.1f }.contains({ mapped.x, mapped.y })) {
                    selected.type = SelectionType::Polygon;
                    selected.polygon.poly = i;
                    selected.polygon.vertex = j;
                    selected.mode = SelectionMode::Position;
                    return;
                }
            }
        }

        selected.mode = SelectionMode::None;

    } else if (const auto mmv = event.getIf<sf::Event::MouseMoved>()) {
        if (selected.mode == SelectionMode::None) { return; }
        const auto mapped = m_transform.transformPoint(window.mapPixelToCoords(mmv->position));

        switch (selected.type) {
        case SelectionType::Circle:
            switch (selected.mode) {
            case SelectionMode::Position:
                m_shapes.circles[selected.circle].circle.pos = nav::Vector2f{ mapped.x, mapped.y };
                m_circle_editors[selected.circle].anchor_pos.setPosition(mapped);
                m_circle_editors[selected.circle].anchor_size.setPosition(mapped + sf::Vector2f(m_shapes.circles[selected.circle].circle.radius, 0));
                m_circle_editors[selected.circle].display.setPosition(mapped);
                break;
            case SelectionMode::Size: {
                const auto pos = m_circle_editors[selected.circle].display.getPosition();
                const auto diff = pos - mapped;
                const auto radius = diff.length();
                m_shapes.circles[selected.circle].circle.radius = radius;
                m_circle_editors[selected.circle].anchor_size.setPosition(pos + sf::Vector2f(radius, 0));
                m_circle_editors[selected.circle].display.setRadius(radius);
                m_circle_editors[selected.circle].display.setOrigin({ radius, radius });
                break; }
            default: break;
            }
            break;
        case SelectionType::Polygon:
            m_shapes.polys[selected.polygon.poly].poly[selected.polygon.vertex] = nav::Vector2f{ mapped.x, mapped.y };
            m_poly_editors[selected.polygon.poly].anchors[selected.polygon.vertex].setPosition(mapped);
            m_poly_editors[selected.polygon.poly].display[selected.polygon.vertex].position = mapped;
            if (selected.polygon.vertex == 0) {
                m_poly_editors[selected.polygon.poly].display.back() = m_poly_editors[selected.polygon.poly].display.front();
            } else if (selected.polygon.vertex == m_poly_editors[selected.polygon.poly].display.size() - 1) {
                m_poly_editors[selected.polygon.poly].display.front() = m_poly_editors[selected.polygon.poly].display.back();
            }
            break;
        }

    } else if (event.is<sf::Event::MouseButtonReleased>()) {
        if (selected.mode == SelectionMode::None) { return; }
        selected.mode = SelectionMode::None;
        compute_mesh();
    }
}

void NavmeshEditor::render(sf::RenderTarget& target, const sf::RenderStates& states) {
    target.draw(m_display.data(), m_display.size(), sf::PrimitiveType::Triangles, states);
    for (const auto& p : m_poly_editors) {
        target.draw(p.display.data(), p.display.size(), sf::PrimitiveType::LineStrip, states);
        for (const auto& a : p.anchors) {
            target.draw(a, states);
        }
    }
    for (const auto& c : m_circle_editors) {
        target.draw(c.display, states);
        target.draw(c.anchor_pos, states);
        target.draw(c.anchor_size, states);
    }
}

