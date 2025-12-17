#include "pch.h"
#include "debugger.h"
#include "util/env.h"
#include "util/deltatime.h"
#include "assets/flags.h"
#include "objects/trigger.h"
#include "world/game.h"
#include "world/scene.h"


SceneDebugger::SceneDebugger()
    : m_font(shmy::env::app_dir() / "calibri.ttf"), m_stats_fps(m_font, "", 22), m_stats_mouse(m_font, "", 22)
{
    m_stats_fps.setPosition({ 20, 20 });
    m_stats_mouse.setPosition({ 20, 46 });
    m_stats_fps.setFillColor(sf::Color::White);
    m_stats_mouse.setFillColor(sf::Color::White);
}

void SceneDebugger::init(const Scene* scene) {
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
    for (const auto& [_, e] : scene->entities) {
        if (e.is_character()) {
            auto& shape = m_colliders.emplace_back();
            shape.setPosition(e.get_trigger_collider().position);
            shape.setRadius(e.get_trigger_collider().radius);
            shape.setOrigin({ e.get_trigger_collider().radius, e.get_trigger_collider().radius });
            shape.setFillColor(sf::Color::Transparent);
            shape.setOutlineThickness(0.015f);
            shape.setOutlineColor(sf::Color::Magenta);
        } else {
            auto& shape = m_boundaries.emplace_back(sf::PrimitiveType::Lines);
            shape.append(sf::Vertex(e.get_sorting_boundary().left, sf::Color::Magenta));
            shape.append(sf::Vertex(e.get_sorting_boundary().right, sf::Color::Magenta));
        }
        auto& shape = m_outlines.emplace_back();
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


void SceneDebugger::update() {
    size_t i = 0;
    size_t j = 0;
    for (const auto& [_, e] : p_scene->entities) {
        if (e.is_character()) {
            m_colliders[j].setPosition(e.get_trigger_collider().position);
            j++;
        }
        m_outlines[i].setPosition(e.get_sprite().getPosition() - e.get_sprite().getOrigin());
        i++;
    }

    i = 0;
    for (const auto& t : p_scene->triggers) {
        FlagTable::Allow = !t.used;
        if (t.condition.evaluate(FlagTable::callback) && !t.cooldown) {
            m_triggers[i].setOutlineColor(sf::Color::Magenta);
        } else {
            m_triggers[i].setOutlineColor(sf::Color(255, 0, 255, 100));
        }
        i++;
    }

    m_motionguide_line.setCount(p_scene->get_player().get_tracker().get_inverse_index() + 1);

    m_stats_fps.setString("FPS: " + std::to_string(Time::framerate()));
}

static std::string round3(float val) {
    auto temp = std::to_string((int)std::abs(std::round(val * 1000.f)));
    if (temp.size() == 1) temp = "000" + temp;
    if (temp.size() == 2) temp = "00" + temp;
    if (temp.size() == 3) temp = "0"  + temp;
    temp.insert(temp.end()-3, '.');
    if (val < 0) temp = "-" + temp;
    return temp;
}

void SceneDebugger::handle_event(const sf::Event& event) {
    if (const auto mmv = event.getIf<sf::Event::MouseMoved>()) {
        auto path = std::vector<sf::Vector2f>();
        for (const auto& p : p_scene->get_player().get_tracker().get_active_path()) { path.push_back({ p.x, p.y }); }
        m_motionguide_line = sfu::LineShape(path);
        m_motionguide_line.setStart(0);

        const auto mapped = p_scene->camera.mapPixelToWorld(mmv->position, p_game->render_settings.viewport);
        const auto world = p_scene->screen_to_world.transformPoint(mapped);
        m_stats_mouse.setString("screen: " +
            std::to_string(mmv->position.x) + ", " + std::to_string(mmv->position.y) + "\nworld: " +
            round3(world.x) + ", " + round3(world.y)
        );
    } else if (event.is<sf::Event::MouseButtonPressed>()) {
        auto path = std::vector<sf::Vector2f>();
        for (const auto& p : p_scene->get_player().get_tracker().get_active_path()) { path.push_back({ p.x, p.y }); }
        m_motionguide_line = sfu::LineShape(path);
        m_motionguide_line.setStart(0);
    } else if (auto kyp = event.getIf<sf::Event::KeyPressed>()) {
        if (kyp->control && kyp->code == sf::Keyboard::Key::F) {
            std::string key;
            std::cout << "query: " << std::flush;
            std::cin >> key;
            if (key == "") return;
            if (FlagTable::has(key)) {
                std::cout << "value: " << FlagTable::get(key, true) << "\n";
            } else {
                std::cout << "key does not exist\n";
            }
        }
    }
}

void SceneDebugger::render_map(sf::RenderTarget& target) const {
    auto states = sf::RenderStates();
    states.transform = p_scene->world_to_screen;
    target.draw(m_pathfinder.data(), m_pathfinder.size(), sf::PrimitiveType::Triangles, states);
    // if (p_scene->get_player().get_tracker().is_moving() || p_scene->gamemode == GameMode::Combat) {
    if (p_game->gamemode == GameMode::Normal || p_game->gamemode == GameMode::Combat) {
        target.draw(m_motionguide_line, p_scene->world_to_screen);
    }
}

void SceneDebugger::render(sf::RenderTarget& target) const {
    for (const auto& t : m_triggers) {
        target.draw(t, p_scene->world_to_screen);
    }
    for (const auto& b : m_boundaries) {
        target.draw(b);
    }
    target.draw(m_xaxis, p_scene->world_to_screen);
    target.draw(m_yaxis, p_scene->world_to_screen);
    target.draw(m_origin_marker, p_scene->world_to_screen);
    for (const auto& c : m_colliders) {
        target.draw(c, p_scene->world_to_screen);
    }
    for (const auto& o : m_outlines) {
        target.draw(o);
    }
}

void SceneDebugger::render_ui(sf::RenderTarget& target) const {
    target.draw(m_stats_fps);
    target.draw(m_stats_mouse);
}

