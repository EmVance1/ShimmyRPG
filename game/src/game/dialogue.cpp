#include "pch.h"
#include "dialogue.h"
#include "flags.h"
#include "game/rules/game_mode.h"


namespace dia = shmy::speech;


void Dialogue::begin(dia::Graph&& graph, GameMode init_mode, const std::string& id) {
    m_id = id;
    m_graph = std::move(graph);
    m_vert = dia::Graph::EXIT;
    for (const auto& e : m_graph.eps) {
        const auto once_id = "once_dia_" + id + "_entry" + std::to_string(e.vert);
        FlagTable::Allow = !FlagTable::has_flag(once_id);
        if (m_graph.eval_expr(e.condition, FlagTable::callback)) {
            FlagTable::set_flag(once_id, 1, false);
            m_vert = e.vert;
            break;
        }
    }
    if (m_vert == dia::Graph::EXIT) {
        std::cerr << "dialogue error - no eligible entry point found\n";
        exit(1);
    }
    m_line = 0;
    m_state = State::Lines;
    m_init_mode = init_mode;
}

void Dialogue::advance(size_t index) {
    if (!m_applied) return;
    m_applied = false;

    switch (m_state) {
    case State::Empty: case State::EmptyWithFollowup:
        throw std::invalid_argument("no active dialogue");

    case State::Player: {
        const auto& resp = m_graph.edges[index];
        m_graph.eval_expr(resp.modifiers, FlagTable::callback);
        if (resp.edge == dia::Graph::EXIT) {
            m_state = State::Empty;
            FlagTable::clear_temps();
        } else {
            m_state = State::Lines;
            m_vert = resp.edge;
            m_line = 0;
        }
        break; }

    case State::Lines: {
        const auto& v = current_vert();
        if (++m_line != v.n_lines) return;

        if (v.n_edges == 0) {
            m_vert = v.edges;
            m_line = 0;
        } else if (v.n_edges == dia::Graph::EXIT && v.edges == 0) {
            m_state = State::Empty;
            FlagTable::clear_temps();
        } else if (v.n_edges == dia::Graph::EXIT) {
            m_state = State::EmptyWithFollowup;
            m_followup = m_graph.strs[v.edges];
            FlagTable::clear_temps();
        } else {
            m_state = State::Player;
        }
        break; }
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
    case State::Empty: case State::EmptyWithFollowup:
        throw std::invalid_argument("no active dialogue");

    case State::Player: {
        const auto& v = current_vert();
        auto sel = Selection{};
        for (size_t i = v.edges; i < v.edges + v.n_edges; i++) {
            if (m_graph.eval_expr(m_graph.edges[i].condition, FlagTable::callback)) {
                sel.push_back(Choice{ &m_graph.strs[m_graph.edges[i].line], i });
            }
        }
        return sel; }

    case State::Lines:
        return Line{ &m_graph.strs[current_vert().speaker], &m_graph.strs[current_vert().lines + m_line] };
    }
    return Line{};
}

