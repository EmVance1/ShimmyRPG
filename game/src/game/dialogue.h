#pragma once
#include <SFML/Graphics.hpp>
#include "scripting/speech_graph.h"
#include "game/game_mode.h"


class Dialogue {
public:
    enum class State {
        Empty,
        EmptyWithFollowup,
        Player,
        Lines,
    };
    struct Line {
        std::string speaker;
        std::string line;
    };
    struct Choice {
        std::string line;
        size_t index;
    };
    using Selection = std::vector<Choice>;
    using Element = std::variant<Line, Selection>;

private:
    std::string m_id;
    SpeechGraph m_graph;
    std::string m_vertex = "entry";
    size_t m_vertex_index = 0;
    State m_state = State::Empty;
    GameMode m_init_mode;
    bool m_unapplied = false;
    std::string m_followup = "";

private:
    SpeechVertex& current_vertex() { return m_graph.at(m_vertex); }
    const SpeechVertex& current_vertex() const { return m_graph.at(m_vertex); }

public:
    Dialogue() = default;

    bool is_playing() const { return m_state != State::Empty && m_state != State::EmptyWithFollowup; }
    State get_state() const { return m_state; }
    GameMode get_init_mode() const { return m_init_mode; }
    std::optional<std::string> get_followup() const { if (m_state != State::EmptyWithFollowup) { return {}; } return m_followup; }

    void begin(SpeechGraph&& graph, GameMode init_mode, const std::string& id);
    void advance(size_t index = 0);
    bool apply_advance();
    Element get_current_element() const;

};

