#include "pch.h"
#include "dialogue.h"
#include "flags.h"


namespace dia = shmy::speech;

static void set_response_flags(const std::unordered_map<std::string, FlagModifier>& flags) {
    for (const auto& [k, v] : flags) {
        FlagTable::change_flag(k, v);
    }
}


void Dialogue::begin(dia::Graph&& graph, GameMode init_mode, const std::string& id) {
    m_id = id;
    m_graph = std::move(graph);
    m_vertex = "";
    for (size_t i = 0; m_graph.contains("entry" + std::to_string(i)); i++) {
        const auto name = "entry" + std::to_string(i);
        const auto once_id = "once_dia_" + id + "_" + name;
        FlagTable::Never = !FlagTable::has_flag(once_id);
        if (m_graph.at(name).conditions.evaluate()) {
            m_vertex = "entry" + std::to_string(i);
            FlagTable::set_flag(once_id, 1, false);
            break;
        }
    }
    if (m_vertex == "") {
        std::cerr << "dialogue error -  no eligible entry point found\n";
        exit(1);
    }
    m_vertex_index = 0;
    m_state = State::Lines;
    m_init_mode = init_mode;
}

void Dialogue::advance(size_t index) {
    if (m_unapplied) { return; }
    m_unapplied = true;
    switch (m_state) {
    case State::Empty: case State::EmptyWithFollowup:
        throw std::exception("no active dialogue");
    case State::Player: {
        const auto& options = std::get<std::vector<dia::Response>>(current_vertex().outcome);
        const auto& response = options[index];
        set_response_flags(response.flags);
        if (response.edge == "exit") {
            m_state = State::Empty;
        } else {
            m_state = State::Lines;
            m_vertex = response.edge;
            m_vertex_index = 0;
        }
        break; }
    case State::Lines:
        m_vertex_index++;
        if (m_vertex_index == current_vertex().lines.size()) {
            if (std::holds_alternative<dia::Exit>(current_vertex().outcome)) {
                m_state = State::Empty;
            } else if (const auto script = std::get_if<dia::ExitInto>(&current_vertex().outcome)) {
                m_followup = script->script;
                m_state = State::EmptyWithFollowup;
            } else if (auto vert = std::get_if<dia::Goto>(&current_vertex().outcome)) {
                m_vertex = vert->vertex;
                m_vertex_index = 0;
            } else {
                m_state = State::Player;
            }
        }
        break;
    }
}


bool Dialogue::apply_advance() {
    if (m_unapplied) {
        m_unapplied = false;
        return true;
    }
    return false;
}

Dialogue::Element Dialogue::get_current_element() const {
    switch (m_state) {
    case State::Empty: case State::EmptyWithFollowup:
        throw std::exception("no active dialogue");
    case State::Player: {
        const auto& options = std::get<std::vector<dia::Response>>(current_vertex().outcome);
        auto vec = std::vector<Choice>();
        for (size_t i = 0; i < options.size(); i++) {
            if (options[i].conditions.evaluate()) {
                vec.push_back({options[i].text, i});
            }
        }
        return Selection{ vec }; }
    case State::Lines:
        return Line{ current_vertex().speaker, current_vertex().lines[m_vertex_index] };
    }
    return Line{};
}

