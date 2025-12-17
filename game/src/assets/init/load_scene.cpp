#include "pch.h"
#include <rapidjson/document.h>
#include "objects/trigger.h"
#include "util/json.h"
#include "util/env.h"
#include "util/iso_map.h"
#include "world/scene.h"
#include "world/game.h"
#include "load_scene.h"
#include "game/events.h"


SceneLoader::SceneLoader(Game* _game, const rapidjson::Value& _prefabs)
    : game(_game), prefabs(_prefabs)
{}


void init_engine_api(lua_State* L);

void SceneLoader::load(Scene* _scene, const std::string& r_id, const std::string& s_id) {
    scene = _scene;
    scene->game = game;
    portals.clear();

    scene->lua_vm.init_env(init_engine_api, "shmy");
    lua_pushlightuserdata(scene->lua_vm.get_state(), scene);
    lua_setfield(scene->lua_vm.get_state(), LUA_REGISTRYINDEX, "_scene");

    const auto dirname = shmy::env::pkg_full() / r_id / s_id;
    const auto doc = shmy::json::load_from_file(dirname / "scene.json");

    scene->name = JSON_GET_STR(doc, "label");
    scene->pathfinder = nav::Mesh::read_file(dirname / "scene.nav");

    const auto scale = scene->pathfinder.display_scale;
    scene->scale = scale;
    scene->motionguide_square.setSize({ 0.08f, 0.08f });
    scene->motionguide_square.setOrigin({ 0.04f, 0.04f });
    scene->motionguide_square.setFillColor(sf::Color::Transparent);
    scene->motionguide_square.setOutlineColor(sf::Color::Cyan);
    scene->motionguide_square.setOutlineThickness(0.015f);

    scene->camera.setRefSize((sf::Vector2f)game->render_settings.viewport, true);
    // if (meta.HasMember("zoom")) {
    //     scene->camera.zoom(meta["zoom"].GetFloat(), sfu::Camera::ZoomFunc::Instant);
    // }

    scene->background.load_from_folder(dirname);
    scene->background.set_frustum_margin(0.5f);
    const auto bgbounds = scene->background.get_bounds();
    scene->origin = bgbounds.position + bgbounds.size * 0.5f;
    scene->world_to_screen = world_to_screen(scene->origin, scale);
    scene->screen_to_world = screen_to_world(scene->origin, scale);

    //  MAYBE RE-ADD THIS IF WE DECIDE ITS TOO MUCH FUCKERY
    /*
    for (const auto& b : JSON_GET_ARRAY(assets, "bundles")) {
        AssetManager::require(b.GetString());
        scene->refs.insert(b.GetString());
    }
    */
    for (const auto& s : JSON_GET_ARRAY(doc, "scripts")) {
        scene->lua_vm.load_file(shmy::env::pkg_full() / s.GetString());
    }

    for (const auto& e : JSON_GET_ARRAY(doc, "entities")) {
        load_entity(e);
    }

    if (scene->player_id == "") { throw std::runtime_error("exactly one entity MUST be designated 'player'\n"); }

    for (const auto& e : JSON_GET_ARRAY(doc, "triggers")) {
        auto& t = scene->triggers.emplace_back();
        t.bounds = (sfu::RotatedFloatRect)shmy::json::into_floatrect(JSON_GET(e, "rect"));
        if (e.HasMember("angle")) {
            t.bounds.angle = sf::degrees(JSON_GET_FLOAT(e, "angle"));
        }
        if (e.HasMember("act_func")) {
            t.action = action::DoString{ JSON_GET_STR(e, "act_func") };
        } else if (e.HasMember("act_event")) {
            t.action = action::DoEvent{ JSON_GET_STR(e, "act_event") };
        } else if (e.HasMember("act_dia")) {
            t.action = action::LoadDia{ JSON_GET_STR(e, "act_dia") };
        } else if (e.HasMember("act_popup")) {
            t.action = action::Popup{ JSON_GET_STR(e, "act_popup") };
        } else if (e.HasMember("act_portal")) {
            const auto& act = e["act_portal"];
            const auto portal = action::Portal{
                JSON_GET_UINT64(act, "index"),
                shmy::json::into_vector2f(JSON_GET(act, "spawnpos")),
                act.HasMember("lock_id") ? JSON_GET_STR(act, "lock_id") : "false"
            };
            portals.push_back(portal.index);
            t.action = portal;
        } else {
            throw std::runtime_error("trigger has no valid action");
        }
        if (e.HasMember("condition")) {
            const auto cond = std::string(JSON_GET_STR(e, "condition"));
            t.condition = shmy::Expr::from_string(cond);
        }
    }

    scene->camera.setTrackingOffset(50.f);
    scene->camera.setCenter(scene->entities[scene->player_id].get_sprite().getPosition(), true);
    scene->camera.setTrackingMode(sfu::Camera::ControlMode::TrackBehind);

#ifdef SHMY_DEBUG
    scene->debugger.init(scene);
#endif

    scene->lua_vm.on_event("OnStart", event_arg::none());
}

const std::vector<size_t>& SceneLoader::get_portals() const {
    return portals;
}

