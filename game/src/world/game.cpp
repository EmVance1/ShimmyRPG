#include "pch.h"
#include "game.h"
#include "util/deltatime.h"
#include "data/mixer.h"


Game::Game(sf::RenderWindow* _window, Viewport& viewport)
    : p_viewport(&viewport),
    render_ctx(_window, sf::FloatRect({ 0.f, 0.f }, (sf::Vector2f)viewport.box.size)),
    gui(gui::Position({0, 0}), (sf::Vector2f)viewport.box.size, gui::Style()),
    cursor(style.cursor_texture)
{
    cursor.setOrigin({ 4, 3 });

#ifdef VANGO_DEBUG
    auto _ = ImGui::SFML::Init(*_window);
    ImGui::GetIO().Fonts->Clear();
    ImGui::GetIO().Fonts->AddFontFromFileTTF("res/CascadiaCode.ttf", 18.f);
    ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    _ = ImGui::SFML::UpdateFontTexture();
#endif
    if (emergencies_only) abort();
    emergencies_only = this;
}

Game::~Game() {
    shmy::data::Mixer::reset();
}

Game* Game::emergencies_only = nullptr;


Entity& Game::entity(uint32_t handle) {
    return m_entities.at(handle);
}
const Entity& Game::entity(uint32_t handle) const {
    return m_entities.at(handle);
}

Entity& Game::entity_by_id(const std::string& id) {
    return m_entities.at(m_entity_map.at(id));
}
const Entity& Game::entity_by_id(const std::string& id) const {
    return m_entities.at(m_entity_map.at(id));
}

uint32_t Game::entity_handle(const std::string& id) const {
    return m_entity_map.at(id);
}

uint32_t Game::entity_hook(const char* id) {
    if (strncmp(id, "Narrator", sizeof("Narrator")) == 0) {
        return UINT32_MAX;
    } else {
        return emergencies_only->m_entity_map.at(id);
    }
}

Entity& Game::player() {
    return m_entities.at(m_active_player);
}
const Entity& Game::player() const {
    return m_entities.at(m_active_player);
}
uint32_t Game::player_id() const {
    return m_active_player;
}


void Game::set_mode(Mode mode, bool cinema_manual, bool cinema_wide) {
    if (mode == gamemode) return;
    render_ctx.cinematic_wide = false;
    gamemode = mode;
    if (mode == Mode::Simulation) {
        player().get_tracker().stop();
        player().get_tracker().start();
        // std::cout << player().get_tracker().get_active_path_length() << "\n";
        normal_mode.show();
        cinematic_mode.hide();
    } else {
        player().get_tracker().stop();
        // std::cout << player().get_tracker().get_active_path_length() << "\n";
        normal_mode.hide();
        cinematic_mode.manual_exit = cinema_manual;
        if (cinema_wide) {
            render_ctx.cinematic_wide = true;
        }
    }
}

void Game::queue_portal(const Portal& portal, uint32_t entity) {
    if (portal.locked) {
        auto pop = gui::Window::create(gui::lo::center({0, 0}), { gui::lo::percent(35), gui::lo::absolute(200) }, style);
        gui.add_widget(pop, true);
        pop->set_position(gui::lo::center({0, 0}));
        pop->set_background_color(sf::Color::Transparent);

        auto msg = gui::Text::create(gui::lo::center({0, 0}), gui::lo::fill(), style, "This door is locked");
        msg->set_text_position(gui::lo::center({0, 0}));
        msg->set_background_texture(sf::IntRect({250, 0}, {50, 50}));
        msg->set_border({2, 2});
        pop->add_widget(msg);
        player().get_tracker().stop();
        m_portal_queue = nullptr;
        return;
    }

    if (entity == m_active_player) {
        m_portal_entity = entity;
        m_portal_queue = &portal;
    } else {
        auto& last = active_scene();
        auto& scene = m_scenes[portal.exit_scene];
        const auto& exit_portal = scene.portals[portal.exit_portal];
        last.entities.erase(std::find(last.entities.begin(), last.entities.end(), entity));
        scene.entities.push_back(entity);
        m_entities[entity].set_navmesh(&scene.pathfinder);
        m_entities[entity].set_position(exit_portal.position, scene.world_to_screen);
        m_entities[entity].busy_reached(Entity::BusyTarget::SceneChange);
    }
}

void Game::exec_scene_swap() {
    if (!m_portal_queue) return;

    Time::stop();
    auto& last = active_scene();
    gui.remove_widget(last.name);
    m_active_scene = m_portal_queue->exit_scene;
    auto& scene = active_scene();
    const auto& exit_portal = scene.portals[m_portal_queue->exit_portal];

    scene.camera.setCenter(scene.world_to_screen.transformPoint(exit_portal.position));
    scene.background.update_soft(scene.camera.getFrustum());
    last.set_sleeping(true);

    last.entities.erase(std::find(last.entities.begin(), last.entities.end(), m_portal_entity));
    scene.entities.push_back(m_portal_entity);
    m_entities[m_portal_entity].set_navmesh(&scene.pathfinder);
    m_entities[m_portal_entity].set_position(exit_portal.position, scene.world_to_screen);
    m_entities[m_portal_entity].busy_reached(Entity::BusyTarget::SceneChange);

    for (const auto& t : last.tracks) {
        if (!scene.tracks.contains(t)) {
            auto opts = shmy::data::Mixer::StopOptions();
            opts.delay_millis = 500;
            opts.fadeout_millis = 500;
            shmy::data::Mixer::stop_track(t, opts);
        }
    }
    for (const auto& t : scene.tracks) {
        auto opts = shmy::data::Mixer::StartOptions();
        opts.loop = true;
        opts.fadein_vol = 1.f;
        opts.fadein_millis = 500;
        shmy::data::Mixer::play_track(t, opts);
    }

    scene.init_gui(gui);
    m_portal_queue = nullptr;

#ifdef VANGO_DEBUG
    m_debugger.init(&scene);
#endif
    scene.set_sleeping(false);
    Time::start();
}


void Game::handle_events() {
#define MAP_CURSOR(vec) (sf::Vector2f(vec - p_viewport->box.position) / p_viewport->scale)

    while (auto event = render_ctx.window->pollEvent()) {
        const auto mapped = event->visit([&](auto&& v) {
            using T = std::decay_t<decltype(v)>;

            if constexpr (std::is_same_v<T, sf::Event::MouseMoved>) {
                mouse_now = MAP_CURSOR(v.position);
                cursor.setPosition(mouse_now);
                return (sf::Event)sf::Event::MouseMoved{ (sf::Vector2i)mouse_now };
            } else if constexpr (std::is_same_v<T, sf::Event::MouseButtonPressed>) {
                return (sf::Event)sf::Event::MouseButtonPressed{ v.button, (sf::Vector2i)MAP_CURSOR(v.position) };
            } else if constexpr (std::is_same_v<T, sf::Event::MouseButtonReleased>) {
                return (sf::Event)sf::Event::MouseButtonReleased{ v.button, (sf::Vector2i)MAP_CURSOR(v.position) };
            } else if constexpr (std::is_same_v<T, sf::Event::MouseWheelScrolled>) {
                return (sf::Event)sf::Event::MouseWheelScrolled{ v.wheel, v.delta, (sf::Vector2i)MAP_CURSOR(v.position) };
            } else if constexpr (std::is_same_v<T, sf::Event::Closed>) {
                render_ctx.window->close();
                return (sf::Event)v;
            } else {
                return *event;
            }
        });

        if (gui.handle_event(mapped)) continue;
        active_scene().handle_input(mapped);
        switch (gamemode) {
        case Mode::Simulation:
            normal_mode.handle_input(mapped);
            break;
        case Mode::Cinematic:
            cinematic_mode.handle_input(mapped);
            break;
        }

#ifdef SHMY_DEBUG
        m_debugger.handle_input(mapped);
#endif
    }

    while (auto event = m_eventqueue.poll_event()) {
        active_scene().handle_event(*event.value());
        switch (gamemode) {
        case Mode::Simulation:
            normal_mode.handle_event(*event.value());
            break;
        case Mode::Cinematic:
            cinematic_mode.handle_event(*event.value());
            break;
        }
    }
}

void Game::update() {
    gui.update();
    for (size_t i = 0; i < m_scenes.size(); i++) {
        m_scenes[i].update();
    }
    switch (gamemode) {
    case Mode::Simulation:
        normal_mode.update();
        break;
    case Mode::Cinematic:
        cinematic_mode.update();
        break;
    }

#ifdef SHMY_DEBUG
    m_debugger.update();
#endif
}

void Game::render(sf::RenderWindow& window, sf::RenderTexture* src, sf::RenderTexture* dst) {

#ifdef VANGO_DEBUG
    src->clear(sf::Color(80, 70, 80));
    active_scene().render(*src, &m_debugger);
#else
    src->clear(sf::Color(10, 10, 10));
    active_scene().render(*src, nullptr);
#endif
    src->display();

    auto quad = sf::Sprite(src->getTexture());
    quad.setOrigin((sf::Vector2f)src->getSize() * 0.5f);
    quad.setPosition((sf::Vector2f)src->getSize() * 0.5f);
    src->setView(src->getDefaultView());
    for (const auto shader : render_ctx.shaders) {
        dst->clear();
        dst->draw(quad, shader);
        dst->display();
        std::swap(src, dst);
        quad.setTexture(src->getTexture());
    }
    quad.setPosition((sf::Vector2f)window.getSize() * 0.5f);
    quad.setScale({ p_viewport->scale, p_viewport->scale });
    quad.setColor(render_ctx.overlay);
    window.draw(quad);

    src->clear(sf::Color::Transparent);
    src->setView(src->getDefaultView());
    src->draw(gui);
    src->draw(cursor);
    src->display();

    quad.setTexture(src->getTexture(), true);
    quad.setColor(sf::Color::White);
    window.draw(quad);

#ifdef VANGO_DEBUG
    ImGui::SFML::Render(window);
#endif
}

