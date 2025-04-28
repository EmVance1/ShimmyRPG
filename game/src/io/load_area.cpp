#include "pch.h"
#include "world/area.h"
#include "world/region.h"
#include "util/str.h"
#include "util/json.h"
#include "algo/iso_map.h"
#include "algo/graph2d.h"
#include "trigger.h"
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

#ifdef DEBUG
    , debugger(id, scale)
#endif
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

    pathfinder = load_grid_from_image(p_region->m_pathmaps.at(id + "_pathmap"));

    for (const auto& e : doc["entities"].GetArray()) {
        load_entity(prefabs, e);
    }

    if (player_id == "") { std::cout << "error parsing entities: must have player controller\n"; throw std::exception(); }

    for (const auto& e : doc["triggers"].GetArray()) {
        auto& t = triggers.emplace_back();
        t.id = e.GetObject()["id"].GetString();
        t.bounds = json_to_floatrect(e.GetObject()["bounds"]);
        t.single_use = e.GetObject()["single_use"].IsTrue();
        const auto action = e.GetObject()["action"].GetObject();
        if (action.HasMember("BeginScript")) {
            t.action = BeginScript{ action["BeginScript"].GetString() };
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
        if (e.GetObject().HasMember("active_if")) {
            const auto cond = std::string(e.GetObject()["active_if"].GetString());
            try {
                const auto expr = flagexpr_from_string(cond);
            } catch (const std::exception& e) {
                std::cout << "error parsing 'active_if': " << e.what() << "\n";
            }
            t.condition = cond;
        }
    }

    camera.zoom(0.9f);
    camera.setTrackingOffset(50.f);
    camera.setCenter(get_player().get_sprite().getPosition(), true);
    camera.setTrackingMode(sfu::Camera::ControlMode::TrackBehind);

    gui.set_style(p_region->get_style());
    gui.set_size(sf::Vector2f(window->getSize()));
    gui.set_background_color(sf::Color::Transparent);
    {
        auto label = gui::Text::create(gui::Position::topcenter({0.f, 50.f}), sf::Vector2f(300, 35), p_region->get_style(), area_label);
        label->set_text_alignment(gui::Alignment::Center);
        label->set_text_padding(0.f);
        gui.add_widget("area_label", label);
    }
    {
        auto speaker = gui::Text::create(
                gui::Position::bottomcenter({350.f, -350.f}),
                sf::Vector2f(300.f, 50.f),
                p_region->get_style(),
                "");
        speaker->set_visible(false);
        speaker->set_character_size(28);
        speaker->set_sorting_layer(-1);
        speaker->set_text_alignment(gui::Alignment::Center);
        gui.add_widget("dialogue_speaker", speaker);
    }
    {
        auto line = gui::Text::create(
                gui::Position::bottomcenter({0.f, -250.f}),
                sf::Vector2f((float)window->getSize().x - 800.f, 100.f),
                p_region->get_style(),
                "");
        line->set_enabled(false);
        line->set_visible(false);
        line->set_text_padding(10.f);
        line->set_character_size(28);
        gui.add_widget("dialogue_lines", line);
    }
    {
        auto choice = gui::ButtonList::create(
                gui::Position::bottomcenter({0.f, -250.f}),
                sf::Vector2f((float)window->getSize().x - 800.f, 50.f),
                p_region->get_style()
            );
        choice->set_enabled(false);
        choice->set_visible(false);
        gui.add_widget("dialogue_choices", choice);
    }

    normal_mode.init(this);
    cinematic_mode.init(this);
    combat_mode.init(this);
    sleep_mode.init(this);

#ifdef DEBUG
    debugger.init(this);
#endif
}

