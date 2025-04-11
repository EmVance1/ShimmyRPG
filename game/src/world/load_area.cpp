#include "pch.h"
#include "area.h"
#include "region.h"
#include "util/str.h"
#include "util/json.h"
#include "algo/iso_map.h"
#include "algo/graph2d.h"
#include "uuid.h"
#include "gui/gui.h"


const sf::RenderWindow* Area::window = nullptr;


Area::Area(const std::string& id, const Region* parent_region, const sf::Vector2f& topleft, float pathscale)
    : p_parent_region(parent_region),
    m_id(id),
    m_pathfinder(load_grid_from_image(parent_region->m_pathmaps.at(id + "_pathmap"))),
    m_topleft(topleft),
    m_background(parent_region->m_textures.at(id + "_background_texture")),
    m_scale(pathscale),
    m_cart_to_iso(cartesian_to_isometric(topleft)),
    m_iso_to_cart(isometric_to_cartesian(topleft)),
    m_camera(sf::FloatRect({0, 0}, {1920, 1080}))

#ifdef DEBUG
    , m_debugger(id, pathscale)
#endif
{
    m_motionguide_square.setFillColor(sf::Color::Cyan);
    m_motionguide_square.setSize({ m_scale, m_scale });
}


void Area::init(const rapidjson::Document& doc) {
    bool has_player = false;

    m_area_label = std::string(doc["world"].GetObject()["area_label"].GetString());

    for (const auto& e : doc["entities"].GetArray()) {
        const auto id = Uuid::generate_v4();
        const auto tex = e.GetObject()["texture"].GetString();
        m_entities[id] = Entity(
                p_parent_region->m_alphamaps.at(tex + std::string("_map")),
                p_parent_region->m_textures.at(tex + std::string("_texture")),
                p_parent_region->m_textures.at(tex + std::string("_outline")),
                id, &m_pathfinder, m_scale, e.GetObject().HasMember("controller")
            );
        const auto pos = e.GetObject()["position"].GetObject();
        if (pos.HasMember("grid")) {
            m_entities[id].set_position(json_to_vector2f(pos["grid"]) * m_scale, m_cart_to_iso);
        } else if (pos.HasMember("world_iso")) {
            m_entities[id].set_sprite_position(json_to_vector2f(pos["world_iso"]));
        }
        if (e.GetObject().HasMember("controller")) {
            const auto con = e.GetObject()["controller"].GetObject();
            if (con["player"].IsTrue()) {
                if (has_player) {
                    std::cout << "CANNOT HAVE 2 PLAYER CONTROLLERS\n";
                } else {
                    m_player_id = id;
                    has_player = true;
                }
            }
            m_entities[id].set_tracker_speed(con["speed"].GetFloat());
        }
        if (e.GetObject().HasMember("boundary")) {
            const auto bry = e.GetObject()["boundary"].GetArray();
            m_entities[id].set_sorting_boundary(json_to_vector2f(bry[0]), json_to_vector2f(bry[1]));
        } else {
            m_entities[id].set_sorting_boundary(sf::Vector2f(0, 0));
        }
        if (e.GetObject().HasMember("script_name")) {
            m_script_lookup[e.GetObject()["script_name"].GetString()] = id;
        }
    }

    if (!has_player) {
        std::cout << "MUST HAVE PLAYER CONTROLLER\n";
    }

    for (const auto& e : doc["triggers"].GetArray()) {
        auto& t = m_triggers.emplace_back();
        t.id = e.GetObject()["id"].GetString();
        t.bounds = json_to_floatrect(e.GetObject()["bounds"]);
        t.single_use = e.GetObject()["single_use"].IsTrue();
        const auto action = e.GetObject()["action"].GetObject();
        if (action.HasMember("BeginScript")) {
            t.action = BeginScript{ action["BeginScript"].GetString() };
        } else if (action.HasMember("GotoRegion")) {
            t.action = GotoRegion{ action["GotoRegion"].GetString() };
        } else if (action.HasMember("GotoArea")) {
            t.action = GotoArea{ action["GotoArea"].GetUint64() };
        }
    }

    m_camera.move({0.f, 100.f});
    m_camera.zoom(0.9f);
    m_camera.setTrackingOffset(100.f);
    m_camera.setTrackingMode(sfu::Camera::ControlMode::TrackBehind);

    m_guistyle.load_from_file("res/styles/wooden.json");
    m_gui.set_style(m_guistyle);
    m_gui.set_size(sf::Vector2f(window->getSize()));
    {
        auto label = gui::Text::create(gui::Position::topcenter({0.f, 50.f}), 25, m_guistyle, m_area_label);
        label->set_background_enabled(true);
        m_gui.add_widget("area_label", label);
    }

#ifdef DEBUG
    m_debugger.init(this);
#endif
}

