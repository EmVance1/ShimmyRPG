#pragma once
#include <SFML/Graphics.hpp>
#include <navmesh/mesh.h>
#include "shape_file.h"


struct PolygonEditor {
    std::vector<sf::Vertex> display;
    std::vector<sf::CircleShape> anchors;
};

struct CircleEditor {
    sf::CircleShape display;
    sf::RectangleShape anchor_pos;
    sf::RectangleShape anchor_size;
};

class NavmeshEditor {
private:
    MeshFile m_shapes;
    nav::Mesh m_mesh;
    std::vector<sf::Vertex> m_display;
    sf::Transform m_transform;

    std::vector<PolygonEditor> m_poly_editors;
    std::vector<CircleEditor>  m_circle_editors;
    enum class SelectionType { Circle, Polygon };
    enum class SelectionMode { None, Position, Size };
    struct {
        SelectionMode mode;
        SelectionType type;
        union {
            size_t circle;
            struct {
                size_t poly;
                size_t vertex;
            } polygon;
        };
    } selected;

private:
    void compute_mesh();
    void compute_cutouts();
    void init_gizmos();

public:
    bool load_file(const std::filesystem::path& path);
    bool save_file(const std::filesystem::path& path);
    void export_file(const std::filesystem::path& path);
    void set_world_scale(float scale) { m_shapes.display_scale = m_mesh.display_scale = scale; }
    float get_world_scale() { return m_shapes.display_scale; }
    void set_world_transform(const sf::Transform& tf) { m_transform = tf; }

    void handle_event(const sf::Event& event, const sf::RenderWindow& window);
    void render(sf::RenderTarget& target, const sf::RenderStates& states);
};

