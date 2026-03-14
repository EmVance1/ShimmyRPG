#include "pch.h"
#include <imgui.h>
#include "debugger.h"
#include "util/deltatime.h"
#include "util/split.h"
#include "util/log.h"
#include "data/flags.h"
#include "objects/trigger.h"
#include "world/game.h"
#include "world/scene.h"


void Debugger::handle_command() {
    if (console[0] == 0) return;
    const auto toks = shmy::core::split(console, ' ');

    if (toks[0] == "flag") {
        if (toks.size() < 3) {
            console_out = "invalid command";
            return;
        } else if (toks[1] == "get") {
            const auto k = toks[2];
            if (!shmy::data::Flags::has(k)) {
                console_out = "invalid key for 'get' command";
                return;
            }
            console_out = k + ": " + std::to_string(shmy::data::Flags::get(k));
        } else if (toks[1] == "set") {
            if (toks.size() < 4) {
                console_out = "invalid command";
                return;
            }
            const auto k = toks[2];
            if (!shmy::data::Flags::has(k)) {
                console_out = "invalid key for 'set' command";
                return;
            }
            const auto v = (uint64_t)std::atoll(toks[3].c_str());
            shmy::data::Flags::set(k, v);
            console_out = k + ": " + toks[3];
        }
    }
}

void Debugger::init(Scene* scene) {
    m_colliders.clear();
    m_boundaries.clear();
    m_outlines.clear();
    m_triggers.clear();
    m_pathfinder.clear();

    p_scene = scene;
    p_game = scene->game;

    for (const auto tri : scene->pathfinder.triangles) {
        const auto b_color = sf::Color((uint8_t)(rand() % 50) + 20, (uint8_t)(rand() % 150) + 105, (uint8_t)(rand() % 150) + 105, 70);
        const auto w_color = sf::Color((uint8_t)(rand() % 55) + 200, (uint8_t)(rand() % 100) + 55, (uint8_t)(rand() % 100) + 55,  70);
        const auto color = tri.weight > 1.f ? w_color : b_color;
        m_pathfinder.push_back(sf::Vertex{ sf::Vector2f(scene->pathfinder.vertices[tri.A].x, scene->pathfinder.vertices[tri.A].y), color });
        m_pathfinder.push_back(sf::Vertex{ sf::Vector2f(scene->pathfinder.vertices[tri.B].x, scene->pathfinder.vertices[tri.B].y), color });
        m_pathfinder.push_back(sf::Vertex{ sf::Vector2f(scene->pathfinder.vertices[tri.C].x, scene->pathfinder.vertices[tri.C].y), color });
    }

    for (const auto& t : scene->triggers) {
        auto& shape = m_triggers.emplace_back();
        shape.setOrigin(t.bounds.size * 0.5f);
        shape.setPosition(t.bounds.position + t.bounds.size * 0.5f);
        shape.setRotation(t.bounds.angle);

        shape.setSize(t.bounds.size);
        shape.setFillColor(sf::Color::Transparent);
        shape.setOutlineThickness(0.015f);
        shape.setOutlineColor(sf::Color::Magenta);
    }
    for (const auto& E : scene->entities) {
        auto e = p_game->entity(E);
        if (e.is_character()) {
            auto& shape = m_colliders[e.id()];
            shape.setPosition(e.get_trigger_collider().position);
            shape.setRadius(e.get_trigger_collider().radius);
            shape.setOrigin({ e.get_trigger_collider().radius, e.get_trigger_collider().radius });
            shape.setFillColor(sf::Color::Transparent);
            shape.setOutlineThickness(0.015f);
            shape.setOutlineColor(sf::Color::Magenta);
        } else {
            auto& shape = m_boundaries[e.id()];
            shape.setPrimitiveType(sf::PrimitiveType::Lines);
            shape.append(sf::Vertex(e.get_sorting_boundary().left, sf::Color::Cyan));
            shape.append(sf::Vertex(e.get_sorting_boundary().right, sf::Color::Cyan));
        }
        auto& shape = m_outlines[e.id()];
        shape.setPosition(e.get_sprite().getPosition() - e.get_sprite().getOrigin());
        shape.setSize(sf::Vector2f(e.get_sprite().getAnimation().getCellSize()));
        shape.setFillColor(sf::Color::Transparent);
        shape.setOutlineThickness(2.f);
        shape.setOutlineColor(sf::Color::Cyan);
    }

    m_origin_marker.setPointCount(8);
    m_origin_marker.setFillColor(sf::Color(255, 160, 0));
    m_origin_marker.setPosition({ 0, 0 });
    const auto out = 0.2f;
    const auto in  = 0.03f;
    m_origin_marker.setPoint(0, {    0, -out });
    m_origin_marker.setPoint(2, {  out,    0 });
    m_origin_marker.setPoint(4, {    0,  out });
    m_origin_marker.setPoint(6, { -out,    0 });
    m_origin_marker.setPoint(1, {   in,  -in });
    m_origin_marker.setPoint(3, {   in,   in });
    m_origin_marker.setPoint(5, {  -in,   in });
    m_origin_marker.setPoint(7, {  -in,  -in });

    m_xaxis.setSize({ 200.f, 0.02f });
    m_xaxis.setOrigin({ 100.f, 0.01f });
    m_xaxis.setFillColor(sf::Color(200, 120, 0));
    m_yaxis.setSize({ 0.02f, 200.f });
    m_yaxis.setOrigin({ 0.01f, 100.f });
    m_yaxis.setFillColor(sf::Color(200, 120, 0));
}


void Debugger::handle_input(const sf::Event& event) {
    ImGui::SFML::ProcessEvent(*(p_game->render_ctx.window), event);

    if (const auto mmv = event.getIf<sf::Event::MouseMoved>()) {
        auto path = std::vector<sf::Vector2f>();
        for (const auto& p : p_game->player().get_tracker().get_active_path()) { path.push_back({ p.x, p.y }); }
        m_motionguide_line = sfu::LineShape(path);
        m_motionguide_line.setStart(0);

        const auto mapped = p_scene->camera.mapPixelToWorld(mmv->position);
        const auto world = p_scene->screen_to_world.transformPoint(mapped);
        cursor_screen = mmv->position;
        cursor_cart = mapped;
        cursor_world = world;
    } else if (event.is<sf::Event::MouseButtonPressed>()) {
        auto path = std::vector<sf::Vector2f>();
        for (const auto& p : p_game->player().get_tracker().get_active_path()) { path.push_back({ p.x, p.y }); }
        m_motionguide_line = sfu::LineShape(path);
        m_motionguide_line.setStart(0);
    } else if (const auto kyp = event.getIf<sf::Event::KeyPressed>()) {
        if (kyp->control && kyp->code == sf::Keyboard::Key::C) {
            console_open = !console_open;
        }
    } else if (event.is<sf::Event::Closed>()) {
        ImGui::SFML::Shutdown();
    }
}

void Debugger::update() {
    ImGui::SFML::Update(*(p_game->render_ctx.window), sf::seconds(shmy::core::Time::deltatime()));

    for (const auto& E : p_scene->entities) {
        auto& e = p_game->entity(E);
        if (e.is_character()) {
            m_colliders[e.id()].setPosition(e.get_trigger_collider().position);
        }
        m_outlines[e.id()].setPosition(e.get_sprite().getPosition() - e.get_sprite().getOrigin());
    }

    size_t i = 0;
    for (const auto& t : p_scene->triggers) {
        shmy::data::Flags::Allow() = !t.used;
        if (t.condition.evaluate(shmy::data::Flags::kv_hook) && !t.cooldown) {
            m_triggers[i].setOutlineColor(sf::Color::Magenta);
        } else {
            m_triggers[i].setOutlineColor(sf::Color(255, 0, 255, 100));
        }
        i++;
    }

    m_motionguide_line.setCount(p_game->player().get_tracker().get_inverse_index() + 1);

    ImGui::PushStyleColor(ImGuiCol_DockingEmptyBg, IM_COL32(0,0,0,0));
    ImGui::PushStyleColor(ImGuiCol_WindowBg, IM_COL32(0,0,0,0));
    ImGui::DockSpaceOverViewport(0, ImGui::GetMainViewport());
    ImGui::PopStyleColor(2);

    ImGui::SetNextWindowBgAlpha(0.3f);
    ImGui::Begin("Diagnostics");
    ImGui::Text("FPS:    %d", shmy::core::Time::framerate());
    if (ImGui::CollapsingHeader("cursor", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::Text("screen: ( %d, %d )", cursor_screen.x, cursor_screen.y);
        ImGui::Text("cart:   ( %d, %d )", (int)cursor_cart.x, (int)cursor_cart.y);
        ImGui::Text("world:  ( %.3f, %.3f )", (double)cursor_world.x, (double)cursor_world.y);
    }
    if (ImGui::CollapsingHeader("scene", ImGuiTreeNodeFlags_DefaultOpen)) {
        if (ImGui::Button("reload")) {
            p_game->reload();
        }
    }
    ImGui::End();

    if (console_open) {
        ImGui::BeginTabBar("Console");
        ImGui::Begin("Terminal");
        if (ImGui::InputText("##command", console, 128, ImGuiInputTextFlags_EscapeClearsAll|ImGuiInputTextFlags_EnterReturnsTrue)) {
            handle_command();
            memset(console, 0, 128);
        }
        ImGui::Text("%s", console_out.c_str());
        ImGui::End();
        ImGui::Begin("Log");
        ImGui::Text("%s", shmy::core::Logger().get_log().c_str());
        ImGui::End();
        ImGui::EndTabBar();
    }
}

void Debugger::render_map(sf::RenderTarget& target) const {
    auto states = sf::RenderStates();
    states.transform = p_scene->world_to_screen;
    target.draw(m_pathfinder.data(), m_pathfinder.size(), sf::PrimitiveType::Triangles, states);
    // if (p_scene->get_player().get_tracker().is_moving() || p_scene->gamemode == Game::Mode::Combat) {
    if (p_game->gamemode == Game::Mode::Simulation) {
        target.draw(m_motionguide_line, p_scene->world_to_screen);
    }
}

void Debugger::render(sf::RenderTarget& target) const {
    for (const auto& t : m_triggers) {
        target.draw(t, p_scene->world_to_screen);
    }
    for (const auto& [_, b] : m_boundaries) {
        target.draw(b);
    }
    target.draw(m_xaxis, p_scene->world_to_screen);
    target.draw(m_yaxis, p_scene->world_to_screen);
    target.draw(m_origin_marker, p_scene->world_to_screen);
    for (const auto& [_, c] : m_colliders) {
        target.draw(c, p_scene->world_to_screen);
    }
    for (const auto& [_, o] : m_outlines) {
        target.draw(o);
    }
}

