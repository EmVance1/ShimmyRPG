#include "pch.h"
#include "scene.h"
#include "util/random.h"
#include "util/deltatime.h"
#include "gui/layout.h"
#include "scripting/lua/runtime.h"
#include "objects/trigger.h"
#include "game.h"
#include "data/flags.h"
#include "data/bundler.h"
#include "config/init/load_scene.h"


Scene::Scene(SceneLoader& loader, const std::string& r_id, const std::string& s_id)
    : camera(sf::FloatRect({0, 0}, {0, 0}))
{
    loader.load(this, r_id, s_id);
}

Scene::Scene(Scene&& other) noexcept : lua_vm(std::move(other.lua_vm)) { std::cerr << "scene object moved\n"; abort(); }

Scene::~Scene() {
    for (const auto& r : refs) {
        shmy::data::Bundler::release(r);
    }
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

void Scene::init_gui(gui::Panel& root) {
    auto scn_pnl = gui::Panel::create(
        gui::Position({ 0.f, 0.f }),
        gui::lo::fill(),
        game->style
    );
    root.add_widget(name, scn_pnl);
    scn_pnl->set_background_color(sf::Color::Transparent);

    auto label = gui::Text::create(
        gui::Position{ gui::lo::xcenter(0), gui::lo::top(30) },
        gui::Sizing({ 300.f, 40.f }),
        game->style, name
    );
    scn_pnl->add_widget("area_label", label);
    label->set_text_position(gui::lo::center({ 0.f, 0.f }));
    label->set_background_texture(sf::IntRect({250, 0}, {50, 50}));
    label->set_border({1, 1});

    for (auto& [id, portal] : portals) {
        portal.icon = gui::Button::create(
            gui::Position({ 0.f, 0.f }),
            gui::Sizing{ gui::lo::absolute(35.f), gui::lo::fitcontent() },
            game->style, "Ex");
        scn_pnl->add_widget(portal.icon);
        portal.icon->set_background_texture(sf::IntRect({250, 0}, {50, 50}));
        portal.icon->set_border({ 2, 2 });
        portal.icon->set_character_size(22);
        portal.icon->set_text_padding(4);
        auto ptr = game;
        portal.icon->set_callback([&, ptr](){ ptr->normal_mode.signal_action(shmy::sim::UsePortalAction{ ptr->player_id(), &portal }); });
    }
}


void Scene::handle_trigger(const Trigger& trigger, uint32_t entity) {
    (void)entity;
    shmy::data::Flags::Allow() = !trigger.used;
    if (!trigger.condition.evaluate(shmy::data::Flags::kv_hook)) return;

    switch (trigger.action.index()) {
    case 0: {
        const auto dostr = std::get<action::DoString>(trigger.action);
        lua_vm.load_anon(dostr.str);
        break; }
    case 1: {
        const auto doevent = std::get<action::DoEvent>(trigger.action);
        lua_vm.on_event(doevent.event, shmy::lua::EventArgs::nil());
        break; }
    case 2: {
        const auto loaddia = std::get<action::LoadDia>(trigger.action);
        game->set_mode(Game::Mode::Cinematic);
        game->cinematic_mode.signal_action(shmy::sim::Cinematic::BeginSpeech{ loaddia.modpath });
        break; }
    case 3: {
        const auto popup = std::get<action::Popup>(trigger.action);
        auto pop = gui::Window::create(gui::lo::center({0, 0}), { gui::lo::percent(35), gui::lo::absolute(200) }, game->style);
        game->gui.add_widget(pop, true);
        pop->set_position(gui::lo::center({0, 0}));
        pop->set_background_color(sf::Color::Transparent);

        auto msg = gui::Text::create(gui::lo::center({0, 0}), gui::lo::fill(), game->style, popup.msg);
        pop->add_widget(msg);
        msg->set_text_position(gui::lo::center({0, 0}));
        msg->set_background_texture(sf::IntRect({250, 0}, {50, 50}));
        msg->set_border({2, 2});
        game->player().get_tracker().stop();
        break; }
    }
}

void Scene::handle_event(const shmy::Event& event) {
    if (sleeping) return;

    (void)event;
}

void Scene::handle_input(const sf::Event& event) {
    if (sleeping) return;

    (void)event;
}

void Scene::update() {
    for (auto  E : entities) {
        auto& e = game->entity(E);
        if (!e.is_offstage()) {
            e.update(world_to_screen);
        }
    }
    lua_vm.update();

    if (sleeping) return;

    sprites_sort();
    camera.update(shmy::core::Time::deltatime());
    background.update(camera.getFrustum());
    if (game->player().get_tracker().is_moving()) {
        const auto vec = game->player().get_tracker().get_target_position();
        motionguide_square.setPosition({ vec.x, vec.y });
    }
}

void Scene::render(sf::RenderTarget& target, const Debugger* debugger) {
    target.setView(camera);
    target.draw(background);

#ifdef SHMY_DEBUG
    debugger->render_map(target);
#endif

    if (game->player().get_tracker().is_moving()) {
        target.draw(motionguide_square, world_to_screen);
    }

    for (auto E : entities) {
        const auto& e = game->entity(E);
        if (!e.is_offstage()) {
            target.draw(e.get_sprite());
            if (e.is_hovered()) {
                target.draw(e.get_outline_sprite());
            }
        }
    }

#ifdef SHMY_DEBUG
    debugger->render(target);
#endif
}

