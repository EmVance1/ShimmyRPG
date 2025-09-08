#include "pch.h"
#include "area.h"
#include "util/str.h"
#include "util/env.h"
#include "sorting.h"
#include "time/deltatime.h"
#include "region.h"
#include "objects/trigger.h"
#include "scripting/lua/script.h"


static std::mt19937 RNG{ std::random_device()() };
RenderSettings* Area::render_settings = nullptr;


Entity& Area::get_player() {
    return entities.at(player_id);
}

const Entity& Area::get_player() const {
    return entities.at(player_id);
}

Entity& Area::get_entity_by_script_id(const std::string& _id) {
    return entities.at(script_to_uuid.at(_id));
}

const Entity& Area::get_entity_by_script_id(const std::string& _id) const {
    return entities.at(script_to_uuid.at(_id));
}

Entity& Area::get_entity_by_story_id(const std::string& _id) {
    return entities.at(story_to_uuid.at(_id));
}

const Entity& Area::get_entity_by_story_id(const std::string& _id) const {
    return entities.at(story_to_uuid.at(_id));
}


void Area::update_motionguide() {
    motionguide_square.setPosition(get_player().get_tracker().get_target_position());
}

void Area::handle_trigger(const Trigger& trigger) {
    FlagTable::Never = !FlagTable::has_flag(trigger.once_id);
    if (!trigger.condition.evaluate()) { return; }
    FlagTable::set_flag(trigger.once_id, 1, false);

    switch (trigger.action.index()) {
    case 0: {
        const auto loadscript = std::get<BeginScript>(trigger.action);
        auto& s = lua_vm.spawn_script(shmy::env::pkg_full() / loadscript.filename);
        s.start();
        break; }
    case 1: {
        const auto loaddia = std::get<BeginDialogue>(trigger.action);
        begin_dialogue(shmy::speech::load_from_file(shmy::env::pkg_full() / loaddia.filename), loaddia.filename);
        set_mode(GameMode::Dialogue);
        break; }
    case 2: {
        const auto popup = std::get<Popup>(trigger.action);
        auto popup_ui = gui::Popup::create(gui::Position::center({0, 0}), sf::Vector2f(700, 150), p_region->get_style(), popup.message);
        popup_ui->set_position(gui::Position::center({0, 0}));
        get_player().get_tracker().stop();
        gui.add_widget("popup", popup_ui);
        break; }
    case 3: {
        // UNIMPLEMENTED: GOTO REGION
        break; }
    case 4: {
        if (suppress_portals) { return; }

        const auto gotoarea = std::get<GotoArea>(trigger.action);
        if (FlagTable::get_flag(gotoarea.lock_id, true)) {
            auto popup_ui = gui::Popup::create(gui::Position::center({0, 0}), sf::Vector2f(700, 150), p_region->get_style(), "This door is locked.");
            popup_ui->set_position(gui::Position::center({0, 0}));
            get_player().get_tracker().stop();
            gui.add_widget("lock_popup", popup_ui);
        } else {
            p_region->set_active_area(gotoarea.index);
            p_region->get_active_area().get_player().set_position(gotoarea.spawnpos, cart_to_iso);
            p_region->get_active_area().suppress_portals = true;
        }
        break; }
    case 5: {
        const auto camerazoom = std::get<CameraZoom>(trigger.action);
        zoom_target = camerazoom.target;
        break; }
    case 6: {
        const auto change = std::get<ChangeFlag>(trigger.action);
        FlagTable::change_flag(change.name, change.mod);
        break; }
    }
}

void Area::begin_dialogue(shmy::speech::Graph&& graph, const std::string& dia_id) {
    cinematic_mode.dialogue.begin(std::move(graph), gamemode, dia_id);
    const auto line = std::get<Dialogue::Line>(cinematic_mode.dialogue.get_current_element());
    auto dia_gui = gui.get_widget<gui::Panel>("dialogue");
    dia_gui->set_enabled(true);
    dia_gui->set_visible(true);
    auto speaker_gui = dia_gui->get_widget<gui::TextWidget>("speaker");
    if (line.speaker == "Narrator") {
        speaker_gui->set_label("Narrator");
    } else {
        speaker_gui->set_label(get_entity_by_script_id(line.speaker).story_id());
    }
    auto line_gui = dia_gui->get_widget<gui::TextWidget>("lines");
    line_gui->set_label(line.line);
}

void Area::begin_combat(
        const std::unordered_set<std::string>& ally_tags,
        const std::unordered_set<std::string>& enemy_tags,
        const std::unordered_set<std::string>& enemysenemy_tags,
        const std::unordered_set<std::string>& unaligned_tags
    )
{
    auto dist = std::uniform_int_distribution<uint32_t>(1, 20);

    combat_mode.participants.clear();
    combat_mode.participants.push_back(CombatParticipant{ player_id, dist(RNG), CombatFaction::Ally });
    get_player().get_tracker().stop();

    for (auto& [name, e] : entities) {
        for (const auto& t : ally_tags) {
            if (e.get_tags().contains(t)) {
                combat_mode.participants.push_back(CombatParticipant{ name, dist(RNG), CombatFaction::Ally });
                e.get_tracker().stop();
                goto end_loop;
            }
        }
        for (const auto& t : enemy_tags) {
            if (e.get_tags().contains(t)) {
                combat_mode.participants.push_back(CombatParticipant{ name, dist(RNG), CombatFaction::Enemy });
                e.get_tracker().stop();
                goto end_loop;
            }
        }
        for (const auto& t : enemysenemy_tags) {
            if (e.get_tags().contains(t)) {
                combat_mode.participants.push_back(CombatParticipant{ name, dist(RNG), CombatFaction::EnemysEnemy });
                e.get_tracker().stop();
                goto end_loop;
            }
        }
        for (const auto& t : unaligned_tags) {
            if (e.get_tags().contains(t)) {
                combat_mode.participants.push_back(CombatParticipant{ name, dist(RNG), CombatFaction::UnalignedHostile });
                e.get_tracker().stop();
                goto end_loop;
            }
        }
end_loop: ;
    }

    std::sort(combat_mode.participants.begin(), combat_mode.participants.end(),
            [](const auto& lhs, const auto& rhs){ return lhs.initiative < rhs.initiative; });

    combat_mode.active_turn = combat_mode.participants.size() - 1;
    combat_mode.advance_turn = true;
}


void Area::set_mode(GameMode mode) {
    if (mode == GameMode::Cinematic && gamemode != GameMode::Cinematic) {
        get_player().get_tracker().stop();
        queued = {};
    } else if (mode != GameMode::Cinematic && gamemode == GameMode::Cinematic) {
        get_player().get_tracker().start();
    } else if (mode == GameMode::Dialogue && gamemode != GameMode::Dialogue) {
        get_player().get_tracker().stop();
    } else if (mode != GameMode::Dialogue && gamemode == GameMode::Dialogue) {
        get_player().get_tracker().start();
    }
    if (mode == GameMode::Cinematic || mode == GameMode::Dialogue) {
        gui.get_widget("tooltip")->set_visible(false);
    }
    if (mode == GameMode::Sleep) {
        background.unload_all();
    } else if (gamemode == GameMode::Sleep) {
        background.update(camera.getFrustum());
        for (auto& t : triggers) {
            t.cooldown = false;
        }
    }
    gamemode = mode;
}



void Area::handle_event(const sf::Event& event) {
    switch (gamemode) {
    case GameMode::Normal:
        normal_mode.handle_event(event);
        break;
    case GameMode::Cinematic: case GameMode::Dialogue:
        cinematic_mode.handle_event(event);
        break;
    case GameMode::Combat:
        combat_mode.handle_event(event);
        break;
    case GameMode::Sleep:
        sleep_mode.handle_event(event);
        break;
    default:
        break;
    }

#ifdef VANGO_DEBUG
    debugger.handle_event(event);
#endif
}

void Area::update() {
    switch (gamemode) {
    case GameMode::Normal:
        normal_mode.update();
        break;
    case GameMode::Cinematic: case GameMode::Dialogue:
        cinematic_mode.update();
        break;
    case GameMode::Combat:
        combat_mode.update();
        break;
    case GameMode::Sleep:
        sleep_mode.update();
        break;
    default:
        break;
    }

    if (gamemode == GameMode::Sleep) { return; }

    sorted_entities = sprites_topo_sort(entities);
    camera.update(Time::deltatime());
    background.update(camera.getFrustum());

    const auto g = FlagTable::get_flag("Player_Coin", true);
    gui.get_widget<gui::Panel>("gold_counter")->get_widget<gui::Text>("goldtxt")->set_label(std::to_string(g));

    const auto crop_thresh = 100.f;
    if (gamemode == GameMode::Cinematic && render_settings->crop.position.y <= crop_thresh) {
        render_settings->crop.position.y += (crop_thresh + 10.f - render_settings->crop.position.y) * Time::deltatime();
        render_settings->crop.size.y -= 2 * (crop_thresh + 10.f - render_settings->crop.position.y) * Time::deltatime();
    } else if (gamemode != GameMode::Cinematic && render_settings->crop.position.y >= 0.f) {
        render_settings->crop.position.y -= (render_settings->crop.position.y + 10.f) * Time::deltatime();
        render_settings->crop.size.y += 2 * (render_settings->crop.position.y + 10.f) * Time::deltatime();
    }

    if (zoom < zoom_target) {
        zoom += Time::deltatime() * 0.5f;
        camera.setSize(zoom * (sf::Vector2f)render_settings->viewport);
        if (zoom >= zoom_target) {
            zoom = zoom_target;
            camera.setSize(zoom * (sf::Vector2f)render_settings->viewport);
        }
    } else if (zoom > zoom_target) {
        zoom -= Time::deltatime() * 0.5f;
        camera.setSize(zoom * (sf::Vector2f)render_settings->viewport);
        if (zoom <= zoom_target) {
            zoom = zoom_target;
            camera.setSize(zoom * (sf::Vector2f)render_settings->viewport);
        }
    }

    gui.update();

#ifdef VANGO_DEBUG
    debugger.update();
#endif
}

void Area::render_world(sf::RenderTarget& target) {
    target.setView(camera);

    target.clear(sf::Color(10, 10, 10));
    target.draw(background);

#ifdef VANGO_DEBUG
    debugger.render_map(target);
#endif

    // if (get_player().get_tracker().is_moving() || gamemode == GameMode::Combat) {
    if (gamemode == GameMode::Combat || gamemode == GameMode::Normal) {
        target.draw(motionguide_square, cart_to_iso);
    }

    for (const auto& e : sorted_entities) {
        if (!e->is_offstage()) {
            target.draw(e->get_sprite());
            if (e->is_hovered()) {
                target.draw(e->get_outline_sprite());
            }
        }
    }
}

void Area::render_overlays(sf::RenderTarget& target) {
#ifdef VANGO_DEBUG
    target.setView(camera);
    debugger.render(target);
#endif

    target.setView(target.getDefaultView());
    target.draw(gui);
}

