#include "pch.h"
#include "area.h"
#include "util/str.h"
#include "sorting.h"
#include "time/deltatime.h"
#include "region.h"
#include "objects/trigger.h"
#include "scripts/lua_script.h"
#include "gui/gui.h"


Entity& Area::get_player() {
    return entities.at(player_id);
}

const Entity& Area::get_player() const {
    return entities.at(player_id);
}


void Area::update_motionguide() {
    const auto& player = get_player();
    if (motionguide_await > 0.05f && motionguide_await < 10.f) {
        motionguide_square.setPosition(player.get_tracker().get_target_position_world());
        motionguide_await = 11.f;
    } else if (motionguide_await < 0.05f) {
        motionguide_await += Time::deltatime();
    }
}

void Area::handle_trigger(const Trigger& trigger) {
    FlagTable::Once = FlagTable::has_flag(trigger.once_id);
    if (!trigger.condition.evaluate()) { return; }
    if (FlagTable::Once) { FlagTable::set_flag(trigger.once_id, 1); }

    switch (trigger.action.index()) {
    case 0: {
        const auto loadscript = std::get<BeginScript>(trigger.action);
        auto& s = scripts.emplace_back(*this);
        s.load_from_file(loadscript.filename);
        break; }
    case 1: {
        const auto loaddia = std::get<BeginDialogue>(trigger.action);
        const auto graph = dialogue_from_file(loaddia.filename);
        begin_dialogue(graph, loaddia.filename);
        set_mode(GameMode::Cinematic, false);
        break; }
    case 2: {
        const auto popup = std::get<Popup>(trigger.action);
        auto popup_ui = gui::Popup::create(gui::Position::center({0, 0}), sf::Vector2f(700, 150), p_region->get_style(), popup.message);
        popup_ui->set_position(gui::Position::center({0, 0}));
        gui.add_widget("popup", popup_ui);
        break; }
    case 4: {
        const auto gotoarea = std::get<GotoArea>(trigger.action);
        p_region->set_active_area(gotoarea.index);
        p_region->get_active_area().get_player().set_position(gotoarea.spawnpos, cart_to_iso);
        p_region->get_active_area().suppress_triggers = gotoarea.suppress_triggers;
        break; }
    }
}

void Area::begin_dialogue(const SpeechGraph& graph, const std::string& dia_id) {
    dialogue.begin(graph, gamemode, dia_id);
    const auto line = std::get<Dialogue::Line>(dialogue.get_current_element());
    auto speaker_gui = gui.get_widget<gui::TextWidget>("dialogue_speaker");
    speaker_gui->set_visible(true);
    if (line.speaker == "Narrator") {
        speaker_gui->set_label("Narrator");
    } else {
        speaker_gui->set_label(story_name_LUT[line.speaker]);
    }
    auto line_gui = gui.get_widget<gui::TextWidget>("dialogue_lines");
    line_gui->set_visible(true);
    line_gui->set_label(line.line);
}

void Area::set_mode(GameMode mode, bool dramatic) {
    if (mode == GameMode::Cinematic && gamemode != GameMode::Cinematic) {
        if (dramatic) { cinematic_timer = 1.5f; }
        get_player().get_tracker().stop();
        queued = {};
    } else if (mode != GameMode::Cinematic && gamemode == GameMode::Cinematic) {
        if (dramatic) { cinematic_timer = 1.5f; }
        get_player().get_tracker().start();
    }
    if (mode == GameMode::Cinematic) {
        gui.get_widget("tooltip")->set_visible(false);
    }
    gamemode = mode;
}



void Area::handle_event(const sf::Event& event) {
    switch (gamemode) {
    case GameMode::Normal:
        normal_mode.handle_event(event);
        break;
    case GameMode::Cinematic:
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

#ifdef DEBUG
    debugger.handle_event(event);
#endif
}

void Area::update() {
    switch (gamemode) {
    case GameMode::Normal:
        normal_mode.update();
        break;
    case GameMode::Cinematic:
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

    update_motionguide();
    background.update(camera.getFrustum());
    sorted_entities = sprites_topo_sort(entities);

    const auto g = FlagTable::get_flag("Player_Coin");
    gui.get_widget<gui::Panel>("gold_counter")->get_widget<gui::Text>("goldtxt")->set_label(std::to_string(g) + "sp");

    if (cinematic_timer > 0.f) {
        cinematic_timer -= Time::deltatime();
        if (gamemode == GameMode::Cinematic) {
            cinemabar_top.move(sf::Vector2f(0.f,  80.f * cinematic_timer * Time::deltatime()));
            cinemabar_bot.move(sf::Vector2f(0.f, -80.f * cinematic_timer * Time::deltatime()));
        } else {
            cinemabar_top.move(sf::Vector2f(0.f, -80.f * cinematic_timer * Time::deltatime()));
            cinemabar_bot.move(sf::Vector2f(0.f,  80.f * cinematic_timer * Time::deltatime()));
        }
    }

#ifdef DEBUG
    debugger.update();
#endif
}

void Area::render(sf::RenderTarget& target) {
    target.setView(camera);

    target.clear(sf::Color(50, 50, 50));
    target.draw(background);

#ifdef DEBUG
    debugger.render_map(target);
#endif

    if (motionguide_await > 0.05f && get_player().get_tracker().is_moving()) {
        target.draw(motionguide_square, cart_to_iso);
    }

    for (const auto& e : sorted_entities) {
        target.draw(e->get_sprite());
        if (e->is_hovered()) {
            target.draw(e->get_outline_sprite());
        }
    }

#ifdef DEBUG
    debugger.render(target);
#endif

    target.setView(target.getDefaultView());
    target.draw(cinemabar_top);
    target.draw(cinemabar_bot);
    target.draw(gui);
}

