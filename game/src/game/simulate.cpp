#include "pch.h"
#include "simulate.h"
#include "world/game.h"
#include "world/scene.h"


namespace shmy::sim {

Entity& Simulation::entity(uint32_t handle) {
    return p_game->entity(handle);
}


void Simulation::set_path_action(const SetPathAction& action) {
    auto& entity = p_game->entity(action.entity);
    entity.get_tracker().set_target_position({ action.target.x, action.target.y });
    entity.get_tracker().start();
    entity.set_busy_until(Entity::BusyTarget::DestReached);
    // std::cout << "moving " << entity.id() << "\n";
}

void Simulation::move_to_action(const MoveToAction& action) {
    auto& scene = p_game->active_scene();
    auto& entity = p_game->entity(action.entity);
    const auto& target = p_game->entity(action.target);
    const auto thresh = 1.f;
    if (target.is_character()) {
        entity.get_tracker().set_target_position(target.get_tracker().get_position());
        entity.get_tracker().trim_path_radial(thresh);
        entity.get_tracker().start();
    } else {
        const auto abs = target.get_sorting_boundary().get_center_of_mass();
        const auto pos = scene.screen_to_world.transformPoint(abs);
        entity.get_tracker().set_target_position({ pos.x, pos.y });
        entity.get_tracker().start();
    }
    entity.set_busy_until(Entity::BusyTarget::DestReached);
}

void Simulation::speak_action(const SpeakAction& action) {
    auto& scene = p_game->active_scene();
    auto& entity = p_game->entity(action.entity);
    const auto& target = p_game->entity(action.target);
    const auto target_pos = target.get_world_position(scene.screen_to_world);
    const auto dist = p_game->player().get_tracker().get_position() - target_pos;
    const float thresh = 1.f;
    if (sf::Vector2f(dist.x, dist.y).lengthSquared() > (thresh * thresh * 1.05f)) {
        move_to_action(MoveToAction{ action.entity, action.target });
        entity.push_action_to_front(action);
        return;
    }

    p_game->cinematic_mode.signal_action(shmy::sim::Cinematic::BeginSpeech{ target.get_dialogue() });
    p_game->set_mode(Game::Mode::Cinematic);
}

void Simulation::examine_action(const ExamineAction& action) {
    auto& scene = p_game->active_scene();
    auto& entity = p_game->entity(action.entity);
    const auto& target = p_game->entity(action.target);
    const auto target_pos = target.get_world_position(scene.screen_to_world);
    const auto dist = p_game->player().get_tracker().get_position() - target_pos;
    const float thresh = 1.f;
    if (sf::Vector2f(dist.x, dist.y).lengthSquared() > (thresh * thresh * 1.05f)) {
        move_to_action(MoveToAction{ action.entity, action.target });
        entity.push_action_to_front(action);
        return;
    }

    p_game->cinematic_mode.signal_action(shmy::sim::Cinematic::BeginSpeech{ target.get_examination() });
    p_game->set_mode(Game::Mode::Cinematic);
}

void Simulation::use_portal_action(const UsePortalAction& action) {
    auto& scene = p_game->active_scene();
    auto& entity = p_game->entity(action.entity);
    const auto entity_pos = entity.get_world_position(scene.screen_to_world);
    const auto dist = action.target->position - sf::Vector2f(entity_pos.x, entity_pos.y);
    const float thresh = 0.4f;
    if (dist.lengthSquared() > (thresh * thresh * 1.05f)) {
        set_path_action(SetPathAction{ action.entity, action.target->position });
        entity.push_action_to_front(action);
        return;
    }
    p_game->queue_portal(*action.target, action.entity);
    entity.set_busy_until(Entity::BusyTarget::SceneChange);
    // std::cout << "transporting " << entity.id() << "\n";
}


void Simulation::init(Game* game) {
    p_game = game;

    root = gui::Panel::create(
        gui::Position({ 0.f, 0.f }),
        gui::lo::fill(),
        game->gui.get_style());
    game->gui.add_widget("explore", root);
    root->set_background_color(sf::Color::Transparent);

    auto tt = gui::Text::create(
        gui::Position{ gui::lo::left(0.f), gui::lo::top(0.f) },
        gui::Sizing{ gui::lo::absolute(120.f), gui::lo::fitcontent() },
        game->gui.get_style(), "");
    root->add_widget("tooltip", tt);
    tooltip = tt.get();
    tooltip->set_text_position({ gui::lo::xcenter(0), gui::lo::top(0) });
    tooltip->set_character_size(22);
    tooltip->set_text_padding(4);
    tooltip->set_background_texture(sf::IntRect({250, 0}, {50, 50}));
    tooltip->set_border({2, 2});
    tooltip->set_enabled(false);
    tooltip->set_visible(false);

    auto cm = gui::VerticalList::create(
        gui::Position{ gui::lo::left(0), gui::lo::top(0) },
        gui::Sizing{ gui::lo::absolute(120.f), gui::lo::absolute(0.f) },
        game->gui.get_style());
    root->add_widget("context_menu", cm);
    ctx_menu = cm.get();
    ctx_menu->set_background_texture(sf::IntRect({250, 0}, {50, 50}));
    ctx_menu->set_border({2, 2});
    ctx_menu->set_enabled(false);
    ctx_menu->set_visible(false);
}

void Simulation::show() {
    root->set_enabled(true);
    root->set_visible(true);
}

void Simulation::hide() {
    root->set_enabled(false);
    root->set_visible(false);
    tooltip->set_enabled(false);
    tooltip->set_visible(false);
    ctx_menu->set_enabled(false);
    ctx_menu->set_visible(false);
}


void Simulation::handle_input(const sf::Event& event) {
    auto& scene = p_game->active_scene();

    if (auto mbp = event.getIf<sf::Event::MouseButtonPressed>()) {
        if (mbp->button == sf::Mouse::Button::Left) {
            const auto mapped = scene.camera.mapPixelToWorld(mbp->position);
            const auto world = scene.screen_to_world.transformPoint(mapped);
            auto& player = p_game->player();
            if (!player.is_hovered()) {
                player.get_tracker().set_target_position({ world.x, world.y });
                player.get_tracker().start();
            }
            signal_action(HideCtxMenu{});

        } else if (mbp->button == sf::Mouse::Button::Right) {
            signal_action(ShowCtxMenu{ p_game->hover_entity, (sf::Vector2f)mbp->position });
        }
    } else if (event.is<sf::Event::MouseMoved>()) {
        signal_action(HideCtxMenu{});

    } else if (auto scrl = event.getIf<sf::Event::MouseWheelScrolled>()) {
        const auto begin = p_game->player().get_sprite().getPosition();
        const auto amt = std::clamp(scene.camera.getZoom() * ((scrl->delta > 0) ? 0.98f : 1.02f), 0.1f, 3.f);
        scene.camera.zoom(amt, sfu::Camera::ZoomFunc::Instant);
        const auto end = p_game->player().get_sprite().getPosition();
        scene.camera.move(begin - end);
    }
}

void Simulation::handle_event(const shmy::Event& event) {
    (void)event;
}

void Simulation::signal_action(const Event& event) {
    std::visit([&](auto&& v) {
        using T = std::decay_t<decltype(v)>;

        auto& scene = p_game->active_scene();

        if constexpr (std::is_same_v<T, ShowCtxMenu>) {
            ctx_menu->clear();
            for (const auto& action : entity(v.entity).get_actions()) {
                auto b = gui::Button::create(sf::Vector2f(), gui::Sizing({ 120, 30 }), scene.game->style, "");
                ctx_menu->add_widget(b);
                b->set_text_padding(5);
                b->set_background_texture(sf::IntRect{ {400, 0}, {50, 50} });
                b->set_style_variant(1);
                b->set_border({2, 2});
                const auto P = p_game->player_id();
                const auto E = v.entity;
                switch (action) {
                case Entity::Action::MoveTo:
                    b->set_label("Move To");
                    b->set_callback([&, P, E](){ move_to_action(MoveToAction{ P, E }); });
                    break;
                case Entity::Action::Speak:
                    b->set_label("Speak");
                    b->set_callback([&, P, E](){ speak_action(SpeakAction{ P, E }); });
                    break;
                case Entity::Action::Examine:
                    b->set_label("Examine");
                    b->set_callback([&, P, E](){ examine_action(ExamineAction{ P, E }); });
                    break;
                default:
                    break;
                }
            }
            ctx_menu->set_position(gui::Position{ gui::lo::left(v.clickpos.x), gui::lo::top(v.clickpos.y) });
            ctx_menu->set_enabled(true);
            ctx_menu->set_visible(true);

        } else if constexpr (std::is_same_v<T, HideCtxMenu>) {
            ctx_menu->set_enabled(false);
            ctx_menu->set_visible(false);

        } else if constexpr (std::is_same_v<T, SetPathAction>) {
            set_path_action(v);
        } else if constexpr (std::is_same_v<T, MoveToAction>) {
            move_to_action(v);
        } else if constexpr (std::is_same_v<T, SpeakAction>) {
            speak_action(v);
        } else if constexpr (std::is_same_v<T, ExamineAction>) {
            examine_action(v);
        } else if constexpr (std::is_same_v<T, UsePortalAction>) {
            use_portal_action(v);
        }
    }, event);
}


void Simulation::update() {
    auto& scene = p_game->active_scene();

    for (const auto E : scene.entities) {
        auto& e = entity(E);
        if (auto action = e.poll_action()) {
            signal_action(*action);
        }
    }

    scene.camera.setTrackingPos(p_game->player().get_sprite().getPosition());

    for (auto& t : scene.triggers) {
        if (p_game->player().get_trigger_collider().intersects(t.bounds)) {
            if (!t.cooldown) {
                scene.handle_trigger(t, Entity::INVALID);
                t.cooldown = true;
                t.used = true;
            }
        } else {
            t.cooldown = false;
        }
    }

    const auto newmap = scene.camera.mapPixelToWorld((sf::Vector2i)p_game->mouse_now);
    if (std::abs(newmap.x - m_oldmap.x) + std::abs(newmap.y - m_oldmap.y) > 0.01f) {
        const auto hover_id = scene.sprites_top(newmap);
        if (hover_id != p_game->hover_entity) {
            if (p_game->hover_entity != Entity::INVALID) {
                entity(p_game->hover_entity).set_hovered(false);
            }
            p_game->hover_entity = hover_id;
            if (hover_id != Entity::INVALID && entity(hover_id).is_interactible()) {
                entity(hover_id).set_hovered(true);
                tooltip->set_label(entity(hover_id).name());
                tooltip->set_visible(true);
            } else {
                tooltip->set_visible(false);
            }
        }

        if (p_game->hover_entity != Entity::INVALID) {
            const auto ttpos = entity(p_game->hover_entity).get_tooltip_position();
            const auto pos = (sf::Vector2f)scene.camera.mapWorldToPixel(ttpos);
            tooltip->set_position(gui::Position(pos - tooltip->get_absolute_size() * 0.5f));
        }

        const auto worldpos = scene.screen_to_world.transformPoint(newmap);
        auto& player = p_game->player();
        if (!player.is_hovered() && !player.get_tracker().is_moving()) {
            player.get_tracker().start();
            if (player.get_tracker().set_target_position({ worldpos.x, worldpos.y })) {
                p_game->cursor.setColor(sf::Color::White);
            } else {
                p_game->cursor.setColor(sf::Color::Red);
            }
            player.get_tracker().pause();
        }

        for (const auto& [_, portal] : scene.portals) {
            if ((worldpos - portal.position).lengthSquared() < 1.f) {
                const auto highpos = scene.world_to_screen.transformPoint(portal.position - sf::Vector2f(0.4f, 0.4f));
                const auto pos = (sf::Vector2f)scene.camera.mapWorldToPixel(highpos);
                portal.icon->set_position(gui::Position(pos - portal.icon->get_absolute_size() * 0.5f));
                portal.icon->set_visible(true);
            } else {
                portal.icon->set_visible(false);
            }
        }
    }
    m_oldmap = newmap;
}

}
