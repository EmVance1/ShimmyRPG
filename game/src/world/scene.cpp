#include "pch.h"
#include "scene.h"
#include "util/env.h"
#include "util/random.h"
#include "util/deltatime.h"
#include "game/events.h"
#include "objects/trigger.h"
#include "game.h"
#include "sorting.h"
#include "assets/flags.h"
#include "assets/manager.h"
#include "assets/init/load_scene.h"


Scene::Scene(SceneLoader& loader, const std::string& r_id, const std::string& s_id)
    : camera(sf::FloatRect({0, 0}, {0, 0}))
{
    loader.load(this, r_id, s_id);
}

Scene::Scene(Scene&& other) noexcept : lua_vm(std::move(other.lua_vm)) {
    std::cerr << "ENGINE ERROR: scene object was moved\n";
    abort();
}

Scene::~Scene() {
    for (const auto& r : refs) {
        AssetManager::release(r);
    }
}


Entity& Scene::get_player() {
    return entities.at(player_id);
}

const Entity& Scene::get_player() const {
    return entities.at(player_id);
}

Entity& Scene::get_entity_by_script_id(const std::string& id) {
    return entities.at(script_to_uuid.at(id));
}

const Entity& Scene::get_entity_by_script_id(const std::string& id) const {
    return entities.at(script_to_uuid.at(id));
}


void Scene::handle_trigger(const Trigger& trigger) {
    FlagTable::Allow = !trigger.used;
    if (!trigger.condition.evaluate(FlagTable::callback)) return;

    switch (trigger.action.index()) {
    case 0: {
        const auto dostr = std::get<action::DoString>(trigger.action);
        lua_vm.load_anon(dostr.str);
        break; }
    case 1: {
        const auto doevent = std::get<action::DoEvent>(trigger.action);
        lua_vm.on_event(doevent.event, event_arg::none());
        break; }
    case 2: {
        const auto loaddia = std::get<action::LoadDia>(trigger.action);
        begin_dialogue(shmy::speech::Graph::load_from_file(shmy::env::pkg_full() / loaddia.file), loaddia.file);
        set_mode(GameMode::Dialogue);
        break; }
    case 3: {
        const auto popup = std::get<action::Popup>(trigger.action);
        auto pop = gui::Popup::create(gui::Position::center({0, 0}), sf::Vector2f(700, 150), game->style, popup.msg);
        pop->set_position(gui::Position::center({0, 0}));
        game->gui.add_widget("popup", pop);
        get_player().get_tracker().stop();
        break; }
    case 4: {
        const auto portal = std::get<action::Portal>(trigger.action);
        if (FlagTable::get(portal.lock_id)) {
            auto pop = gui::Popup::create(gui::Position::center({0, 0}), sf::Vector2f(700, 150), game->style, "This door is locked.");
            pop->set_position(gui::Position::center({0, 0}));
            game->gui.add_widget("lock_popup", pop);
            get_player().get_tracker().stop();
        } else {
            game->queue_scene_swap((int)portal.index, portal.spawnpos);
        }
        break; }
    }
}

void Scene::begin_dialogue(shmy::speech::Graph&& graph, const std::string& dia_id) {
    game->cinematic_mode.dialogue.begin(std::move(graph), game->gamemode, dia_id);
    const auto line = std::get<Dialogue::Line>(game->cinematic_mode.dialogue.get_current_element());
    auto dia_gui = game->gui.get_widget<gui::Panel>("dialogue");
    dia_gui->set_enabled(true);
    dia_gui->set_visible(true);

    auto speaker_gui = dia_gui->get_widget<gui::TextWidget>("speaker");
    if (*line.speaker == "Narrator") {
        speaker_gui->set_label("Narrator");
    } else {
        const auto& e = get_entity_by_script_id(*line.speaker);
        speaker_gui->set_label(e.name());
    }

    auto line_gui = dia_gui->get_widget<gui::TextWidget>("lines");
    line_gui->set_label(*line.line);
}

void Scene::begin_combat(
        const std::unordered_set<std::string>& ally_tags,
        const std::unordered_set<std::string>& enemy_tags,
        const std::unordered_set<std::string>& enemysenemy_tags,
        const std::unordered_set<std::string>& unaligned_tags
    )
{
    game->combat_mode.participants.clear();
    game->combat_mode.participants.push_back(CombatParticipant{ player_id, Random::d20(), CombatFaction::Ally });
    get_player().get_tracker().stop();

    for (auto& [id, e] : entities) {
        for (const auto& t : ally_tags) {
            if (e.get_tags().contains(t)) {
                game->combat_mode.participants.push_back(CombatParticipant{ id, Random::d20(), CombatFaction::Ally });
                e.get_tracker().stop();
                goto contd;
            }
        }
        for (const auto& t : enemy_tags) {
            if (e.get_tags().contains(t)) {
                game->combat_mode.participants.push_back(CombatParticipant{ id, Random::d20(), CombatFaction::Enemy });
                e.get_tracker().stop();
                goto contd;
            }
        }
        for (const auto& t : enemysenemy_tags) {
            if (e.get_tags().contains(t)) {
                game->combat_mode.participants.push_back(CombatParticipant{ id, Random::d20(), CombatFaction::EnemysEnemy });
                e.get_tracker().stop();
                goto contd;
            }
        }
        for (const auto& t : unaligned_tags) {
            if (e.get_tags().contains(t)) {
                game->combat_mode.participants.push_back(CombatParticipant{ id, Random::d20(), CombatFaction::UnalignedHostile });
                e.get_tracker().stop();
                goto contd;
            }
        }
contd: ;
    }

    std::sort(game->combat_mode.participants.begin(), game->combat_mode.participants.end(),
            [](const auto& lhs, const auto& rhs){ return lhs.initiative < rhs.initiative; });

    game->combat_mode.active_turn = game->combat_mode.participants.size() - 1;
    game->combat_mode.advance_turn = true;
}


void Scene::set_mode(GameMode mode) {
    if (mode == GameMode::Cinematic && game->gamemode != GameMode::Cinematic) {
        get_player().get_tracker().stop();
        queued = {};
    } else if (mode != GameMode::Cinematic && game->gamemode == GameMode::Cinematic) {
        get_player().get_tracker().start();
    } else if (mode == GameMode::Dialogue && game->gamemode != GameMode::Dialogue) {
        get_player().get_tracker().stop();
    } else if (mode != GameMode::Dialogue && game->gamemode == GameMode::Dialogue) {
        get_player().get_tracker().start();
    }
    if (mode == GameMode::Cinematic || mode == GameMode::Dialogue) {
        game->gui.get_widget("tooltip")->set_visible(false);
    }
    game->gamemode = mode;
}

void Scene::set_sleeping(bool _sleeping) {
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



void Scene::handle_event(const sf::Event& event) {
    if (sleeping) return;

    switch (game->gamemode) {
    case GameMode::Normal:
        game->normal_mode.handle_event(event);
        break;
    case GameMode::Cinematic: case GameMode::Dialogue:
        game->cinematic_mode.handle_event(event);
        break;
    case GameMode::Combat:
        game->combat_mode.handle_event(event);
        break;
    }

#ifdef SHMY_DEBUG
    debugger.handle_event(event);
#endif
}

void Scene::update() {
    for (auto& [_, e] : entities) {
        if (!e.is_offstage()) {
            e.update(world_to_screen);
            if (e.get_tracker().reached_dest()) {
                // std::cout << "ello?\n";
                // lua_vm.on_event("OnEntityDestinationReached", event_arg::reached_dest(lua_vm, e));
            }
        }
    }
    lua_vm.update();

    if (sleeping) return;

    switch (game->gamemode) {
    case GameMode::Normal:
        game->normal_mode.update();
        break;
    case GameMode::Cinematic: case GameMode::Dialogue:
        game->cinematic_mode.update();
        break;
    case GameMode::Combat:
        game->combat_mode.update();
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
    game->gui.get_widget<gui::Panel>("gold_counter")->get_widget<gui::Text>("goldtxt")->set_label(std::to_string(g));

    const auto crop_thresh = 100.f;
    if (game->gamemode == GameMode::Cinematic && game->render_settings.crop.position.y <= crop_thresh) {
        game->render_settings.crop.position.y += (crop_thresh + 10.f - game->render_settings.crop.position.y) * Time::deltatime();
        game->render_settings.crop.size.y -= 2 * (crop_thresh + 10.f - game->render_settings.crop.position.y) * Time::deltatime();
    } else if (game->gamemode != GameMode::Cinematic && game->render_settings.crop.position.y >= 0.f) {
        game->render_settings.crop.position.y -= (game->render_settings.crop.position.y + 10.f) * Time::deltatime();
        game->render_settings.crop.size.y += 2 * (game->render_settings.crop.position.y + 10.f) * Time::deltatime();
    }

#ifdef SHMY_DEBUG
    debugger.update();
#endif
}

void Scene::render(sf::RenderTarget& target) {
    target.setView(camera);

    target.draw(background);

#ifdef SHMY_DEBUG
    debugger.render_map(target);
#endif

    if (get_player().get_tracker().is_moving()) {
        target.draw(motionguide_square, world_to_screen);
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

