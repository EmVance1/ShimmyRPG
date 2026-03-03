#include "pch.h"
#include <rapidjson/document.h>
#include "objects/trigger.h"
#include "core/split.h"
#include "util/json.h"
#include "util/env.h"
#include "util/iso_map.h"
#include "world/scene.h"
#include "world/game.h"
#include "load_scene.h"
#include "config/lua/lua_api.h"
#include "log.h"
#include "validation.h"


SceneLoader::SceneLoader(Game* _game, const rapidjson::Value& _chars, const rapidjson::Value& _props)
    : game(_game), chars(_chars), props(_props)
{}


void SceneLoader::load(Scene* _scene, const std::string& r_id, const std::string& s_id) {
    scene = _scene;
    scene->game = game;
    portals.clear();
    const auto ctx = "in scene " + r_id + ":" + s_id;

    scene->lua_vm.init_env(init_scene_api, "shmy");
    init_entity_api(scene->lua_vm.get_state());
    init_audio_api(scene->lua_vm.get_state());
    lua_pushlightuserdata(scene->lua_vm.get_state(), game);
    lua_setfield(scene->lua_vm.get_state(), LUA_REGISTRYINDEX, "_game");

    const auto dirname = shmy::env::pkg_full() / "world" / r_id / s_id;
    const auto doc = shmy::json::load_from_file(dirname / "scene.json");
    const auto cfg = validate_scene_config(doc);

    scene->name = cfg.label;
    scene->pathfinder = nav::Mesh::read_file(dirname / "scene.nav");

    const auto scale = scene->pathfinder.display_scale;
    scene->scale = scale;
    scene->motionguide_square.setSize({ 0.08f, 0.08f });
    scene->motionguide_square.setOrigin({ 0.04f, 0.04f });
    scene->motionguide_square.setFillColor(sf::Color::Transparent);
    scene->motionguide_square.setOutlineColor(sf::Color::Cyan);
    scene->motionguide_square.setOutlineThickness(0.015f);

    scene->camera.setRefSize((sf::Vector2f)game->p_viewport->box.size, true);
    scene->camera.setTrackingMode(sfu::Camera::ControlMode::TrackBehind);
    scene->camera.setTrackingOffset(50.f);
    scene->camera.zoom(cfg.zoom, sfu::Camera::ZoomFunc::Instant);

    scene->background.load_from_folder(dirname);
    scene->background.set_frustum_margin(0.5f);
    const auto bgbounds = scene->background.get_bounds();
    scene->origin = bgbounds.position + bgbounds.size * 0.5f;
    scene->world_to_screen = world_to_screen(scene->origin, scale);
    scene->screen_to_world = screen_to_world(scene->origin, scale);

    for (const auto& s : cfg.scripts) {
        scene->lua_vm.load_file(shmy::env::pkg_full() / s);
    }

    for (const auto& t : cfg.tracks) {
        scene->tracks.emplace(t);
    }

    for (const auto& P : cfg.portals) {
        auto& p = scene->portals[P.id];
        p.position = P.position;
        const auto exit = shmy::core::split(P.exit, '.');
        p.exit_scene  = game->m_scene_map[exit[0]];
        p.exit_portal = exit[1];
        p.mouseover_radius = P.mouseover_radius;
        p.preload_radius = P.preload_radius;
        p.locked = P.locked;
    }

    for (const auto& E : doc["triggers"].GetArray()) {
        auto& t = scene->triggers.emplace_back();
        t.bounds = (sfu::RotatedFloatRect)shmy::json::into_floatrect(JSON_GET(E, "rect"));
        if (E.HasMember("angle")) {
            t.bounds.angle = sf::degrees(JSON_GET_FLOAT(E, "angle"));
        }
        if (E.HasMember("act_func")) {
            t.action = action::DoString{ JSON_GET_STR(E, "act_func") };
        } else if (E.HasMember("act_event")) {
            t.action = action::DoEvent{  JSON_GET_STR(E, "act_event") };
        } else if (E.HasMember("act_dia")) {
            t.action = action::LoadDia{  JSON_GET_STR(E, "act_dia") };
        } else if (E.HasMember("act_popup")) {
            t.action = action::Popup{    JSON_GET_STR(E, "act_popup") };
        } else {
            ::shmy::Logger::error(ctx, ": invalid entity stub type");
            scene->triggers.pop_back();
            continue;
        }
        if (E.HasMember("condition")) {
            const auto cond = std::string(JSON_GET_STR(E, "condition"));
            t.condition = shmy::Expr::from_string(cond);
        }
    }

    for (const auto& E : doc["entities"].GetArray()) {
        if (E.HasMember("character")) {
            load_entity(E, JSON_GET_STR(E, "character"), EntityType::Character);
        } else if (E.HasMember("prop")) {
            load_entity(E, JSON_GET_STR(E, "prop"), EntityType::Prop);
        } else {
            ::shmy::Logger::error(ctx, ": invalid entity stub type");
        }
    }

    scene->lua_vm.on_event("OnStart", shmy::lua::EventArgs::nil());
}

const std::vector<size_t>& SceneLoader::get_portals() const {
    return portals;
}

