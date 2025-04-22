#include "pch.h"
#include "dialogue.h"
#include "flags.h"
#include "scripting/speech_graph.h"


static void set_response_flags(const std::unordered_map<std::string, FlagModifier>& flags) {
    for (const auto& [k, v] : flags) {
        FlagTable::change_flag(k, v);
    }
}


void Dialogue::begin(const SpeechGraph& graph, GameMode init_mode) {
    m_graph = graph;
    m_vertex = "entry0";
    for (size_t i = 0; graph.contains("entry" + std::to_string(i)); i++) {
        const auto name = "entry" + std::to_string(i);
        if (graph.at(name).conditions.evaluate()) {
            m_vertex = "entry" + std::to_string(i);
        }
    }
    m_vertex_index = 0;
    m_state = State::Lines;
    m_init_mode = init_mode;
}

void Dialogue::advance(size_t index) {
    if (m_unapplied) { return; }
    m_unapplied = true;
    switch (m_state) {
    case State::Empty:
        throw std::exception("no active dialogue");
    case State::Player: {
        const auto& options = std::get<std::vector<SpeechResponse>>(current_vertex().outcome);
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
            if (std::holds_alternative<SpeechExit>(current_vertex().outcome)) {
                m_state = State::Empty;
            } else if (auto vert = std::get_if<SpeechGoto>(&current_vertex().outcome)) {
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
    case State::Empty:
        throw std::exception("no active dialogue");
    case State::Player: {
        const auto& options = std::get<std::vector<SpeechResponse>>(current_vertex().outcome);
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

