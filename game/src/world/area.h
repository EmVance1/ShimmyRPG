#pragma once
#include <SFML/Graphics.hpp>
#include <rapidjson/document.h>
#include <sfutil/camera.h>
#include <unordered_map>
#include "algo/graph2d.h"
#include "graphics/partialline.h"
#include "entity.h"
#include "trigger.h"
#include "debugger.h"
#include "gui/panel.h"


class Region;

class Area {
private:
    const Region* p_parent_region;
    const std::string m_id;
    std::string m_area_label;

    SpatialGraph2d m_pathfinder;
    sf::Vector2f m_topleft;
    sf::Sprite m_background;
    float m_scale;

    const sf::Transform m_cart_to_iso;
    const sf::Transform m_iso_to_cart;

    std::unordered_map<std::string, Entity> m_entities;
    std::unordered_map<std::string, std::string> m_script_lookup;
    std::vector<Entity*> m_sorted_entities;
    std::string m_player_id;

    std::vector<Trigger> m_triggers;

    sf::RectangleShape m_motionguide_square;
    PartialLine m_motionguide_line;
    float m_motionguide_await = 11.f;

    sfu::Camera m_camera;

    gui::Panel m_gui;
    gui::Style m_guistyle;

#ifdef DEBUG
    AreaDebugView m_debugger;
#endif

private:
    void update_motionguide();
    void handle_trigger(const Trigger& trigger);

public:
    static const sf::RenderWindow* window;

    Area(const std::string& id, const Region* parent_region, const sf::Vector2f& topleft, float pathscale);

    void init(const rapidjson::Document& doc);
    void update();
    void handle_event(const sf::Event& event);
    void render(sf::RenderTarget& target);

    friend class AreaDebugView;
};

