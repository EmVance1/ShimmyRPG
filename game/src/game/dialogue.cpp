#include "pch.h"
#include "dialogue.h"
#include "assets/flags.h"
#include "game/rules/game_mode.h"


namespace dia = shmy::speech;


void Dialogue::begin(dia::Graph&& graph, GameMode init_mode, const std::string& id) {
    m_id = id;
    m_graph = std::move(graph);
    auto vert = dia::Graph::EXIT;
    for (const auto& e : m_graph.eps) {
        const auto once_id = "once_dia_" + id + "_entry" + std::to_string(e.vert);
        FlagTable::Allow = !FlagTable::has(once_id);
        if (m_graph.eval_expr(e.condition, FlagTable::callback)) {
            FlagTable::set(once_id, 1, false);
            vert = e.vert;
            break;
        }
    }
    if (vert == dia::Graph::EXIT) {
        std::cerr << "dialogue error - no eligible entry point found\n";
        exit(1);
    }
    m_vert = m_graph.nth_vertex(vert);
    m_line = m_vert.first_line();
    m_state = State::Lines;
    m_init_mode = init_mode;
}

void Dialogue::advance(dia::Edge resp) {
    if (!m_applied) return;
    m_applied = false;

    switch (m_state) {
    case State::Empty:
        throw std::invalid_argument("no active dialogue");

    case State::Player:
        switch (resp.outcome()) {
        case dia::Edge::Outcome::Hook:
            m_sethook = resp.get_hook();
            m_hashook = true;
            break;
        case dia::Edge::Outcome::Modifier:
            resp.eval_modifiers(FlagTable::callback);
            break;
        default:
            break;
        }
        if (resp.next_vertex().is_exit()) {
            m_state = State::Empty;
            FlagTable::clear_temps();
        } else {
            m_state = State::Lines;
            m_vert = resp.next_vertex();
            m_line = m_vert.first_line();
        }
        break;

    case State::Lines:
        m_line = m_line.next();
        if (m_line != m_vert.end_line()) return;

        switch (m_vert.outcome()) {
        case dia::Vertex::Outcome::Edges:
            m_state = State::Player;
            break;
        case dia::Vertex::Outcome::Goto:
            m_vert = m_vert.get_goto();
            m_line = m_vert.first_line();
            break;
        case dia::Vertex::Outcome::Exit:
            m_state = State::Empty;
            FlagTable::clear_temps();
            break;
        case dia::Vertex::Outcome::ExitHook:
            m_state = State::Empty;
            FlagTable::clear_temps();
            m_hashook = true;
            m_sethook = m_vert.get_hook();
            break;
        }
        break;
    }
}


bool Dialogue::apply_advance() {
    if (!m_applied) {
        m_applied = true;
        return true;
    }
    return false;
}

Dialogue::Element Dialogue::get_current_element() const {
    switch (m_state) {
    case State::Empty:
        throw std::invalid_argument("no active dialogue");

    case State::Player: {
        auto sel = Selection{};
        for (auto e = m_vert.first_edge(); e != m_vert.end_edge(); e = e.next()) {
            if (e.eval_condition(FlagTable::callback)) {
                sel.push_back(e);
            }
        }
        return sel; }

    case State::Lines:
        return Line{ &m_vert.speaker(), &m_line.line() };
    }
    return Line{};
}

