#include "pch.h"
#include <rapidjson/document.h>
#include "objects/trigger.h"
#include "util/json.h"
#include "util/env.h"
#include "util/iso_map.h"
#include "world/area.h"
#include "world/region.h"
#include "load_scene.h"


Area::Area() : camera(sf::FloatRect({0, 0}, {0, 0})), lua_vm("shmy") {}

Area::Area(Area&& other) : lua_vm(std::move(other.lua_vm))
{
    std::cerr << "Area objects should never be copied or moved\n";
    abort();
}


SceneLoader::SceneLoader(Region* _region, const rapidjson::Value& _prefabs)
    : region(_region), prefabs(_prefabs)
{}


void init_engine_api(lua_State* L);

void SceneLoader::load(Area* _area, const std::string& _id) {
    area = _area;
    area->region = region;

    init_engine_api(area->lua_vm.get_state());
    lua_pushlightuserdata(area->lua_vm.get_state(), area);
    lua_setfield(area->lua_vm.get_state(), LUA_REGISTRYINDEX, "_scene");

    const auto filename = shmy::env::pkg_full() / region->id() / (_id + ".json");
    const auto doc = shmy::json::load_from_file(filename);
    const auto& meta = JSON_GET(doc, "world");

    area->name = JSON_GET_STR(meta, "label");
    area->topleft = shmy::json::into_vector2f(JSON_GET_ARRAY(meta, "topleft"));
    const auto scale = JSON_GET_FLOAT(meta, "scale");
    area->pathfinder = nav::Mesh::read_file(shmy::env::pkg_full() / region->id() / (_id + ".nav"), scale);

    area->cart_to_iso = cartesian_to_isometric(area->topleft);
    area->iso_to_cart = isometric_to_cartesian(area->topleft);

    area->motionguide_square.setSize({ scale * 2, scale * 2 });
    area->motionguide_square.setOrigin({ scale, scale });
    area->motionguide_square.setFillColor(sf::Color::Transparent);
    area->motionguide_square.setOutlineColor(sf::Color::Cyan);
    area->motionguide_square.setOutlineThickness(1);

    area->camera.setRefSize((sf::Vector2f)region->render_settings->viewport, true);
    if (meta.HasMember("zoom")) {
        area->camera.zoom(meta["zoom"].GetFloat(), sfu::Camera::ZoomFunc::Instant);
    }

    area->background.load_from_json(JSON_GET(doc, "background"), 0.5f);

    for (const auto& e : JSON_GET_ARRAY(doc, "entities")) {
        load_entity(e);
    }

    if (area->player_id == "") { throw std::runtime_error("exactly one entity MUST be designated 'player'\n"); }

    for (const auto& e : JSON_GET_ARRAY(doc, "triggers")) {
        auto& t = area->triggers.emplace_back();
        t.bounds = (sfu::RotatedFloatRect)shmy::json::into_floatrect(JSON_GET(e, "rect"));
        if (e.HasMember("angle")) {
            t.bounds.angle = sf::degrees(JSON_GET_FLOAT(e, "angle"));
        }
        if (e.HasMember("act_func")) {
            t.action = action::DoString{ JSON_GET_STR(e, "act_func") };
        } else if (e.HasMember("act_script")) {
            t.action = action::LoadScript{ JSON_GET_STR(e, "act_script") };
        } else if (e.HasMember("act_dia")) {
            t.action = action::LoadDia{ JSON_GET_STR(e, "act_dia") };
        } else if (e.HasMember("act_popup")) {
            t.action = action::Popup{ JSON_GET_STR(e, "act_popup") };
        } else if (e.HasMember("act_portal")) {
            const auto& act = e["act_portal"];
            t.action = action::Portal{
                JSON_GET_UINT64(act, "index"),
                shmy::json::into_vector2f(JSON_GET(act, "spawnpos")),
                act.HasMember("lock_id") ? JSON_GET_STR(act, "lock_id") : "false"
            };
        } else {
            throw std::runtime_error("trigger has no valid action");
        }
        if (e.HasMember("condition")) {
            const auto cond = std::string(JSON_GET_STR(e, "condition"));
            t.condition = shmy::Expr::from_string(cond);
        }
    }

    area->camera.setTrackingOffset(50.f);
    area->camera.setCenter(area->entities[area->player_id].get_sprite().getPosition(), true);
    area->camera.setTrackingMode(sfu::Camera::ControlMode::TrackBehind);

#ifdef VANGO_DEBUG
    area->debugger.init(area);
#endif
}

