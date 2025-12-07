#include "pch.h"
#include "area.h"
#include "objects/trigger.h"
#include "util/deltatime.h"
#include "util/random.h"
#include "util/env.h"
#include "flags.h"
#include "region.h"
#include "sorting.h"


Entity& Area::get_player() {
    return entities.at(player_id);
}

const Entity& Area::get_player() const {
    return entities.at(player_id);
}

Entity& Area::get_entity_by_script_id(const std::string& id) {
    return entities.at(script_to_uuid.at(id));
}

const Entity& Area::get_entity_by_script_id(const std::string& id) const {
    return entities.at(script_to_uuid.at(id));
}


void Area::handle_trigger(const Trigger& trigger) {
    FlagTable::Allow = !trigger.used;
    if (!trigger.condition.evaluate(FlagTable::callback)) return;

    switch (trigger.action.index()) {
    case 0: {
        const auto dostr = std::get<action::DoString>(trigger.action);
        lua_vm.load_anon(dostr.str);
        break; }
    case 1: {
        const auto doevent = std::get<action::DoEvent>(trigger.action);
        lua_vm.on_event(doevent.event, -1);
        break; }
    case 2: {
        const auto loaddia = std::get<action::LoadDia>(trigger.action);
        begin_dialogue(shmy::speech::Graph::load_from_file(shmy::env::pkg_full() / loaddia.file), loaddia.file);
        set_mode(GameMode::Dialogue);
        break; }
    case 3: {
        const auto popup = std::get<action::Popup>(trigger.action);
        auto pop = gui::Popup::create(gui::Position::center({0, 0}), sf::Vector2f(700, 150), region->style(), popup.msg);
        pop->set_position(gui::Position::center({0, 0}));
        region->m_gui.add_widget("popup", pop);
        get_player().get_tracker().stop();
        break; }
    case 4: {
        const auto portal = std::get<action::Portal>(trigger.action);
        if (FlagTable::get(portal.lock_id)) {
            auto pop = gui::Popup::create(gui::Position::center({0, 0}), sf::Vector2f(700, 150), region->style(), "This door is locked.");
            pop->set_position(gui::Position::center({0, 0}));
            region->m_gui.add_widget("lock_popup", pop);
            get_player().get_tracker().stop();
        } else {
            region->queue_scene_swap((int)portal.index, portal.spawnpos);
        }
        break; }
    }
}

void Area::begin_dialogue(shmy::speech::Graph&& graph, const std::string& dia_id) {
    region->cinematic_mode.dialogue.begin(std::move(graph), region->gamemode, dia_id);
    const auto line = std::get<Dialogue::Line>(region->cinematic_mode.dialogue.get_current_element());
    auto dia_gui = region->m_gui.get_widget<gui::Panel>("dialogue");
    dia_gui->set_enabled(true);
    dia_gui->set_visible(true);
    auto speaker_gui = dia_gui->get_widget<gui::TextWidget>("speaker");
    if (*line.speaker == "Narrator") {
        speaker_gui->set_label("Narrator");
    } else {
        speaker_gui->set_label(get_entity_by_script_id(*line.speaker).story_id());
    }
    auto line_gui = dia_gui->get_widget<gui::TextWidget>("lines");
    line_gui->set_label(*line.line);
}

void Area::begin_combat(
        const std::unordered_set<std::string>& ally_tags,
        const std::unordered_set<std::string>& enemy_tags,
        const std::unordered_set<std::string>& enemysenemy_tags,
        const std::unordered_set<std::string>& unaligned_tags
    )
{
    region->combat_mode.participants.clear();
    region->combat_mode.participants.push_back(CombatParticipant{ player_id, Random::d20(), CombatFaction::Ally });
    get_player().get_tracker().stop();

    for (auto& [id, e] : entities) {
        for (const auto& t : ally_tags) {
            if (e.get_tags().contains(t)) {
                region->combat_mode.participants.push_back(CombatParticipant{ id, Random::d20(), CombatFaction::Ally });
                e.get_tracker().stop();
                goto contd;
            }
        }
        for (const auto& t : enemy_tags) {
            if (e.get_tags().contains(t)) {
                region->combat_mode.participants.push_back(CombatParticipant{ id, Random::d20(), CombatFaction::Enemy });
                e.get_tracker().stop();
                goto contd;
            }
        }
        for (const auto& t : enemysenemy_tags) {
            if (e.get_tags().contains(t)) {
                region->combat_mode.participants.push_back(CombatParticipant{ id, Random::d20(), CombatFaction::EnemysEnemy });
                e.get_tracker().stop();
                goto contd;
            }
        }
        for (const auto& t : unaligned_tags) {
            if (e.get_tags().contains(t)) {
                region->combat_mode.participants.push_back(CombatParticipant{ id, Random::d20(), CombatFaction::UnalignedHostile });
                e.get_tracker().stop();
                goto contd;
            }
        }
contd: ;
    }

    std::sort(region->combat_mode.participants.begin(), region->combat_mode.participants.end(),
            [](const auto& lhs, const auto& rhs){ return lhs.initiative < rhs.initiative; });

    region->combat_mode.active_turn = region->combat_mode.participants.size() - 1;
    region->combat_mode.advance_turn = true;
}


void Area::set_mode(GameMode mode) {
    if (mode == GameMode::Cinematic && region->gamemode != GameMode::Cinematic) {
        get_player().get_tracker().stop();
        queued = {};
    } else if (mode != GameMode::Cinematic && region->gamemode == GameMode::Cinematic) {
        get_player().get_tracker().start();
    } else if (mode == GameMode::Dialogue && region->gamemode != GameMode::Dialogue) {
        get_player().get_tracker().stop();
    } else if (mode != GameMode::Dialogue && region->gamemode == GameMode::Dialogue) {
        get_player().get_tracker().start();
    }
    if (mode == GameMode::Cinematic || mode == GameMode::Dialogue) {
        region->m_gui.get_widget("tooltip")->set_visible(false);
    }
    region->gamemode = mode;
}

void Area::set_sleeping(bool _sleeping) {
    sleeping = _sleeping;
    if (sleeping) {
        background.unload_all();
    } else {
        background.update(camera.getFrustum());
        for (auto& t : triggers) {
            t.cooldown = false;
        }
    }
}



void Area::handle_event(const sf::Event& event) {
    if (sleeping) return;

    switch (region->gamemode) {
    case GameMode::Normal:
        region->normal_mode.handle_event(event);
        break;
    case GameMode::Cinematic: case GameMode::Dialogue:
        region->cinematic_mode.handle_event(event);
        break;
    case GameMode::Combat:
        region->combat_mode.handle_event(event);
        break;
    }

#ifdef VANGO_DEBUG
    debugger.handle_event(event);
#endif
}

void Area::update() {
    for (auto& [_, e] : entities) {
        if (!e.is_offstage()) {
            e.update(cart_to_iso);
        }
    }
    lua_vm.update();

    if (sleeping) return;

    switch (region->gamemode) {
    case GameMode::Normal:
        region->normal_mode.update();
        break;
    case GameMode::Cinematic: case GameMode::Dialogue:
        region->cinematic_mode.update();
        break;
    case GameMode::Combat:
        region->combat_mode.update();
        break;
    }

    sorted_entities = sprites_topo_sort(entities);
    camera.update(Time::deltatime());
    background.update(camera.getFrustum());
    if (get_player().get_tracker().is_moving()) {
        const auto vec = get_player().get_tracker().get_target_position();
        motionguide_square.setPosition({ vec.x, vec.y });
    }

    const auto g = FlagTable::get("Player.Coin", true);
    region->m_gui.get_widget<gui::Panel>("gold_counter")->get_widget<gui::Text>("goldtxt")->set_label(std::to_string(g));

    const auto crop_thresh = 100.f;
    if (region->gamemode == GameMode::Cinematic && region->render_settings->crop.position.y <= crop_thresh) {
        region->render_settings->crop.position.y += (crop_thresh + 10.f - region->render_settings->crop.position.y) * Time::deltatime();
        region->render_settings->crop.size.y -= 2 * (crop_thresh + 10.f - region->render_settings->crop.position.y) * Time::deltatime();
    } else if (region->gamemode != GameMode::Cinematic && region->render_settings->crop.position.y >= 0.f) {
        region->render_settings->crop.position.y -= (region->render_settings->crop.position.y + 10.f) * Time::deltatime();
        region->render_settings->crop.size.y += 2 * (region->render_settings->crop.position.y + 10.f) * Time::deltatime();
    }

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

    if (get_player().get_tracker().is_moving()) {
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
    target.draw(region->m_gui);
}

