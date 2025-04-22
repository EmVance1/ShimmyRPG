#include "pch.h"
#include "area.h"
#include "region.h"
#include "util/str.h"
#include "util/json.h"
#include "algo/iso_map.h"
#include "algo/graph2d.h"
#include "uuid.h"
#include "gui/gui.h"
#include "trigger.h"
#include "scripts/lua_script.h"


const sf::RenderWindow* Area::window = nullptr;


Area::Area(const std::string& _id, Region* parent_region, const sf::Vector2f& _topleft, float pathscale)
    : p_region(parent_region),
    id(_id),
    pathfinder(load_grid_from_image(parent_region->m_pathmaps.at(id + "_pathmap"))),
    topleft(_topleft),
    background(parent_region->m_textures.at(id + "_background_texture")),
    scale(pathscale),
    cart_to_iso(cartesian_to_isometric(topleft)),
    iso_to_cart(isometric_to_cartesian(topleft)),
    camera(sf::FloatRect({0, 0}, {1920, 1080})),
    gui(gui::Position::topleft({0, 0}), sf::Vector2f(window->getSize()), parent_region->get_style())

#ifdef DEBUG
    , debugger(id, pathscale)
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


void Area::init(const rapidjson::Document& doc) {
    bool has_player = false;

    area_label = std::string(doc["world"].GetObject()["area_label"].GetString());

    for (const auto& e : doc["entities"].GetArray()) {
        const auto e_id = Uuid::generate_v4();
        const auto tex = e.GetObject()["texture"].GetString();
        entities[e_id] = Entity(
                p_region->m_alphamaps.at(tex + std::string("_map")),
                p_region->m_textures.at(tex + std::string("_texture")),
                p_region->m_textures.at(tex + std::string("_outline")),
                e_id, &pathfinder, scale, e.GetObject().HasMember("controller")
            );
        auto& entity = entities[e_id];
        const auto pos = e.GetObject()["position"].GetObject();
        if (pos.HasMember("grid")) {
            entity.set_position(json_to_vector2f(pos["grid"]) * scale, cart_to_iso);
        } else if (pos.HasMember("world_iso")) {
            entity.set_sprite_position(json_to_vector2f(pos["world_iso"]));
        }
        if (e.GetObject().HasMember("controller")) {
            const auto con = e.GetObject()["controller"].GetObject();
            entity.get_tracker().set_speed(con["speed"].GetFloat());
        }
        if (e.GetObject().HasMember("boundary")) {
            const auto bry = e.GetObject()["boundary"].GetArray();
            entity.set_sorting_boundary(json_to_vector2f(bry[0]), json_to_vector2f(bry[1]));
        } else {
            entity.set_sorting_boundary(sf::Vector2f(0, 0));
        }
        if (e.GetObject().HasMember("ids")) {
            const auto ids = e.GetObject()["ids"].GetObject();
            script_name_LUT[ids["script"].GetString()] = e_id;
            dialogue_name_LUT[ids["script"].GetString()] = ids["dialogue"].GetString();
        }
        if (e.GetObject().HasMember("dialogue")) {
            const auto dia = e.GetObject()["dialogue"].GetObject();
            if (dia.HasMember("file")) {
                entity.set_dialogue(dia["file"].GetString());
            } else {
                entity.set_dialogue(dia["line"].GetString());
            }
        }
        auto taglist = std::unordered_set<std::string>();
        for (const auto& tag : e.GetObject()["tags"].GetArray()) {
            taglist.emplace(tag.GetString());
        }

        if (taglist.contains("player")) {
            if (has_player) {
                std::cout << "CANNOT HAVE 2 PLAYER CONTROLLERS\n";
            } else {
                player_id = e_id;
                has_player = true;
            }
        } else {
            entity.get_actions().emplace_back(MoveToAction{});
            entity.get_actions().emplace_back(AttackAction{});
            if (taglist.contains("npc"))         { entity.get_actions().emplace_back(SpeakAction{}); }
            if (taglist.contains("door"))        { entity.get_actions().emplace_back(OpenDoorAction{}); }
            if (taglist.contains("chest"))       { entity.get_actions().emplace_back(OpenInvAction{}); }
            if (taglist.contains("simple_lock")) { entity.get_actions().emplace_back(LockpickAction{}); }
            if (taglist.contains("carryable"))   { entity.get_actions().emplace_back(PickUpAction{}); }
        }
    }


    if (!has_player) {
        std::cout << "MUST HAVE PLAYER CONTROLLER\n";
    }

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
                std::cout << "active_if: " << e.what() << "\n";
            }
            t.condition = cond;
        }
    }

    camera.move({0.f, 100.f});
    camera.zoom(0.9f);
    camera.setTrackingOffset(50.f);
    camera.setTrackingMode(sfu::Camera::ControlMode::TrackAhead);

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

