#include "pch.h"
#include "world/area.h"
#include "world/region.h"
#include "util/str.h"
#include "util/json.h"
#include "algo/iso_map.h"
#include "objects/trigger.h"
#include "scripts/lua_script.h"
#include "gui/gui.h"


const sf::RenderWindow* Area::window = nullptr;


Area::Area(const std::string& _id, Region* parent_region, const sf::Vector2f& _topleft, float _scale)
    : p_region(parent_region),
    id(_id),
    topleft(_topleft),
    scale(_scale),
    cart_to_iso(cartesian_to_isometric(topleft)),
    iso_to_cart(isometric_to_cartesian(topleft)),
    camera(sf::FloatRect({0, 0}, sf::Vector2f(window->getSize()))),
    gui(gui::Position::topleft({0, 0}), sf::Vector2f(window->getSize()), parent_region->get_style())
{
    motionguide_square.setFillColor(sf::Color::Transparent);
    motionguide_square.setOutlineColor(sf::Color::Cyan);
    motionguide_square.setOutlineThickness(1);
    motionguide_square.setSize({ scale * 2, scale * 2 });
    motionguide_square.setOrigin({ scale, scale });

    const auto winsize = window->getSize();
    cinemabar_top.setPosition({0, -150.f});
    cinemabar_top.setSize({(float)winsize.x, 150});
    cinemabar_top.setFillColor(sf::Color::Black);
    cinemabar_bot.setPosition({0, (float)winsize.y});
    cinemabar_bot.setSize({(float)winsize.x, 150});
    cinemabar_bot.setFillColor(sf::Color::Black);
}


void Area::init(const rapidjson::Value& prefabs, const rapidjson::Document& doc) {
    area_label = std::string(doc["world"].GetObject()["area_label"].GetString());

    background.load_from_json(doc["background"]);

    pathfinder = nav::NavMesh::read_file("res/maps/" + id + ".nav", scale);

    for (const auto& e : doc["entities"].GetArray()) {
        load_entity(prefabs, e);
    }

    if (player_id == "") { std::cout << "error parsing entities: must have player controller\n"; throw std::exception(); }

    for (const auto& e : doc["triggers"].GetArray()) {
        auto& t = triggers.emplace_back();
        t.id = e.GetObject()["id"].GetString();
        t.once_id = "once_trig_" + id + t.id;
        t.bounds = (sfu::RotatedFloatRect)json_to_floatrect(e.GetObject()["bounds"]);
        if (e.GetObject().HasMember("angle")) {
            t.bounds.angle = sf::degrees(e.GetObject()["angle"].GetFloat());
        }
        const auto action = e.GetObject()["action"].GetObject();
        if (action.HasMember("BeginScript")) {
            t.action = BeginScript{ action["BeginScript"].GetString() };
        } else if (action.HasMember("BeginDialogue")) {
            t.action = BeginDialogue{ action["BeginDialogue"].GetString() };
        } else if (action.HasMember("Popup")) {
            t.action = Popup{ action["Popup"].GetString() };
        } else if (action.HasMember("GotoRegion")) {
            t.action = GotoRegion{ action["GotoRegion"].GetString() };
        } else if (action.HasMember("GotoArea")) {
            t.action = GotoArea{
                action["GotoArea"].GetObject()["index"].GetUint64(),
                json_to_vector2f(action["GotoArea"].GetObject()["spawnpos"]),
                action["GotoArea"].GetObject()["suppress_triggers"].IsTrue(),
            };
        }
        if (e.GetObject().HasMember("condition")) {
            const auto cond = std::string(e.GetObject()["condition"].GetString());
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

