#include "pch.h"
#include <rapidjson/document.h>
#include "util/uuid.h"
#include "world/area.h"
#include "world/region.h"
#include "util/str.h"
#include "util/json.h"
#include "util/env.h"
#include "util/iso_map.h"
#include "objects/trigger.h"


void init_engine_api(lua_State* L);


Area::Area(const std::string& _id, Region* parent_region)
    : p_region(parent_region), id(_id),
    camera(sf::FloatRect({0, 0}, (sf::Vector2f)render_settings->viewport)),
    gui(gui::Position::topleft({0, 0}), sf::Vector2f(render_settings->viewport), parent_region->get_style())
{
    motionguide_square.setFillColor(sf::Color::Transparent);
    motionguide_square.setOutlineColor(sf::Color::Cyan);
    motionguide_square.setOutlineThickness(1);

    lua_vm = luaL_newstate();
    luaL_openlibs(lua_vm);
    init_engine_api(lua_vm);
    lua_pushlightuserdata(lua_vm, this);
    lua_setfield(lua_vm, LUA_REGISTRYINDEX, "_area");
}

Area::Area(Area&& other) : gui(std::move(other.gui)) {
    std::cout << "Area objects should never be copied or moved\n";
    exit(1);
}

Area::~Area() {
    lua_close(lua_vm);
}


void Area::init(const rapidjson::Value& prefabs, const rapidjson::Document& doc) {
    const auto& meta = JSON_GET(doc, "world");

    story_id = std::string(JSON_GET_STR(meta, "label"));
    topleft = shmy::json::into_vector2f(JSON_GET_ARRAY(meta, "topleft"));
    const auto scale = JSON_GET_FLOAT(meta, "scale");
    pathfinder = nav::Mesh::read_file((shmy::env::pkg_full() / p_region->m_id / id).concat(".nav"), scale);

    cart_to_iso = cartesian_to_isometric(topleft);
    iso_to_cart = isometric_to_cartesian(topleft);

    motionguide_square.setSize({ scale * 2, scale * 2 });
    motionguide_square.setOrigin({ scale, scale });

    if (meta.HasMember("zoom")) {
        camera.setSize(meta["zoom"].GetFloat() * (sf::Vector2f)render_settings->viewport);
    }

    background.load_from_json(JSON_GET(doc, "background"), 0.5f);

    for (const auto& e : JSON_GET_ARRAY(doc, "entities")) {
        load_entity(prefabs, e);
    }

    if (player_id == "") { throw std::invalid_argument("exactly one entity MUST be designated 'player'\n"); }

    for (const auto& e : JSON_GET_ARRAY(doc, "triggers")) {
        auto& t = triggers.emplace_back();
        t.once_id = "once_trig_" + id + shmy::Uuid::generate_v4();
        t.bounds = (sfu::RotatedFloatRect)shmy::json::into_floatrect(JSON_GET(e, "rect"));
        if (e.HasMember("angle")) {
            t.bounds.angle = sf::degrees(JSON_GET_FLOAT(e, "angle"));
        }
        const auto& action = JSON_GET(e, "action");
        if (action.HasMember("BeginScript")) {
            t.action = BeginScript{ JSON_GET_STR(action, "BeginScript") };
        } else if (action.HasMember("BeginDialogue")) {
            t.action = BeginDialogue{ JSON_GET_STR(action, "BeginDialogue") };
        } else if (action.HasMember("Popup")) {
            t.action = Popup{ JSON_GET_STR(action, "Popup") };
        } else if (action.HasMember("GotoRegion")) {
            t.action = GotoRegion{ JSON_GET_STR(action, "GotoRegion") };
        } else if (action.HasMember("GotoArea")) {
            const auto& act = action["GotoArea"];
            t.action = GotoArea{
                JSON_GET_UINT64(act, "index"),
                shmy::json::into_vector2f(JSON_GET(act, "spawnpos")),
                act.HasMember("lock_id") ? JSON_GET_STR(act, "lock_id") : "false"
            };
        } else if (action.HasMember("CameraZoom")) {
            t.action = CameraZoom{ JSON_GET_FLOAT(action, "CameraZoom") };
        } else if (action.HasMember("CameraZoom")) {
            const auto& act = action["ChangeFlag"];
            if (act.HasMember("Add")) {
                t.action = ChangeFlag{ JSON_GET_STR(act, "name"), FlagAdd{ JSON_GET_INT(act, "Add"), true } };
            } else if (action.HasMember("Sub")) {
                t.action = ChangeFlag{ JSON_GET_STR(act, "name"), FlagSub{ JSON_GET_INT(act, "Sub"), true } };
            } else if (action.HasMember("Set")) {
                t.action = ChangeFlag{ JSON_GET_STR(act, "name"), FlagSet{ JSON_GET_INT(act, "Set"), true } };
            }
        }
        if (e.HasMember("condition")) {
            const auto cond = std::string(JSON_GET_STR(e, "condition"));
            try {
                t.condition = shmy::FlagExpr::from_string(cond);
            } catch (const std::exception& e) {
                std::cout << "error parsing 'condition': " << e.what() << "\n";
            }
        }
    }

    camera.zoom(0.9f);
    camera.setTrackingOffset(50.f);
    camera.setCenter(get_player().get_sprite().getPosition(), true);
    camera.setTrackingMode(sfu::Camera::ControlMode::TrackBehind);

    load_gui();

    normal_mode.init(this);
    cinematic_mode.init(this);
    combat_mode.init(this);
    sleep_mode.init(this);

#ifdef DEBUG
    debugger.init(this);
#endif
}

