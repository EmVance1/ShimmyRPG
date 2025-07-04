#include "pch.h"
#include "rapidjson/document.h"
#include "util/uuid.h"
#include "world/area.h"
#include "world/region.h"
#include "util/str.h"
#include "util/json.h"
#include "algo/iso_map.h"
#include "objects/trigger.h"
#include "scripts/lua_script.h"
#include "gui/gui.h"
#include "json_debug.h"


const sf::RenderWindow* Area::window = nullptr;


Area::Area(const std::string& _id, Region* parent_region)
    : p_region(parent_region), id(_id), overlaycolor(sf::Color::White),
    camera(sf::FloatRect({0, 0}, (sf::Vector2f)window->getSize())),
    gui(gui::Position::topleft({0, 0}), sf::Vector2f(window->getSize()), parent_region->get_style())
{
    // std::cout << "constructed area\n";
    motionguide_square.setFillColor(sf::Color::Transparent);
    motionguide_square.setOutlineColor(sf::Color::Cyan);
    motionguide_square.setOutlineThickness(1);

    const auto winsize = window->getSize();
    cinemabar_top.setPosition({0, -150.f});
    cinemabar_top.setSize({(float)winsize.x, 150});
    cinemabar_top.setFillColor(sf::Color::Black);
    cinemabar_bot.setPosition({0, (float)winsize.y});
    cinemabar_bot.setSize({(float)winsize.x, 150});
    cinemabar_bot.setFillColor(sf::Color::Black);
}

Area::Area(Area&& other) :
    p_region(std::move(other.p_region)),
    id(std::move(other.id)),
    area_label(std::move(other.area_label)),
    background(std::move(other.background)),
    pathfinder(std::move(other.pathfinder)),
    topleft(std::move(other.topleft)),
    scale(std::move(other.scale)),
    zoom(std::move(other.zoom)),
    zoom_target(std::move(other.zoom_target)),
    overlaycolor(std::move(other.overlaycolor)),
    cart_to_iso(std::move(other.cart_to_iso)),
    iso_to_cart(std::move(other.iso_to_cart)),
    entities(std::move(other.entities)),
    script_name_LUT(std::move(other.script_name_LUT)),
    story_name_LUT(std::move(other.story_name_LUT)),
    sorted_entities(std::move(other.sorted_entities)),
    player_id(std::move(other.player_id)),
    scripts(std::move(other.scripts)),
    triggers(std::move(other.triggers)),
    suppress_triggers(std::move(other.suppress_triggers)),
    suppress_portals(std::move(other.suppress_portals)),
    motionguide_square(std::move(other.motionguide_square)),
    queued(std::move(other.queued)),
    camera(std::move(other.camera)),
    gui(std::move(other.gui)),
    gamemode(std::move(other.gamemode)),
    normal_mode(std::move(other.normal_mode)),
    cinematic_mode(std::move(other.cinematic_mode)),
    combat_mode(std::move(other.combat_mode)),
    sleep_mode(std::move(other.sleep_mode)),
    cinemabar_top(std::move(other.cinemabar_top)),
    cinemabar_bot(std::move(other.cinemabar_bot)),
    cinematic_timer(std::move(other.cinematic_timer)),
    dialogue(std::move(other.dialogue))

#ifdef DEBUG
    , debugger(std::move(other.debugger))
#endif
{
    // std::cout << "moved area\n";
}


void Area::init(const rapidjson::Value& prefabs, const rapidjson::Document& doc, bool active) {
    const auto& meta = JSON_GET(doc, "world");

    area_label = std::string(JSON_GET_STR(meta, "label"));
    topleft = json_to_vector2f(JSON_GET_ARRAY(meta, "topleft"));
    scale = JSON_GET_FLOAT(meta, "scale");
    pathfinder = nav::NavMesh::read_file(p_region->m_id + id + ".nav", scale);

    cart_to_iso = cartesian_to_isometric(topleft);
    iso_to_cart = isometric_to_cartesian(topleft);

    motionguide_square.setSize({ scale * 2, scale * 2 });
    motionguide_square.setOrigin({ scale, scale });

    if (meta.HasMember("zoom")) {
        camera.setSize(meta["zoom"].GetFloat() * (sf::Vector2f)window->getSize());
    }

    if (active) {
        background.load_from_json(JSON_GET(doc, "background"), 0.2f, camera.getFrustum());
    } else {
        background.load_from_json(JSON_GET(doc, "background"), 0.2f);
    }

    for (const auto& e : JSON_GET_ARRAY(doc, "entities")) {
        load_entity(prefabs, e);
    }

    if (player_id == "") { throw std::invalid_argument("exactly one entity MUST be designated 'player'\n"); }

    for (const auto& e : JSON_GET_ARRAY(doc, "triggers")) {
        auto& t = triggers.emplace_back();
        t.once_id = "once_trig_" + id + Uuid::generate_v4();
        t.bounds = (sfu::RotatedFloatRect)json_to_floatrect(JSON_GET(e, "rect"));
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
                json_to_vector2f(JSON_GET(act, "spawnpos")),
                JSON_IS_TRUE(act, "suppress_triggers"),
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
                t.condition = FlagExpr::from_string(cond);
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

