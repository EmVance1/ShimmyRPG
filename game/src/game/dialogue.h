#pragma once
#include <SFML/Graphics.hpp>
#include "scripting/speech/graph.h"


enum class GameMode;

class Dialogue {
public:
    enum class State {
        Empty,
        EmptyWithFollowup,
        Player,
        Lines,
    };
    struct Line {
        const std::string* speaker;
        const std::string* line;
    };
    struct Choice {
        const std::string* line;
        size_t index;
    };
    using Selection = std::vector<Choice>;
    using Element = std::variant<Line, Selection>;

private:
    std::string m_id;
    GameMode m_init_mode;

    shmy::speech::Graph m_graph;
    State m_state = State::Empty;
    bool  m_applied = true;
    uint32_t m_vert = UINT32_MAX;
    uint32_t m_line = 0;
    std::string m_followup = "";

private:
    shmy::speech::Vert& current_vert() { return m_graph.verts.at(m_vert); }
    const shmy::speech::Vert& current_vert() const { return m_graph.verts.at(m_vert); }

public:
    Dialogue() = default;

    bool is_playing() const { return m_state != State::Empty && m_state != State::EmptyWithFollowup; }
    State get_state() const { return m_state; }
    GameMode get_init_mode() const { return m_init_mode; }
    std::optional<std::string> get_followup() const { if (m_state != State::EmptyWithFollowup) { return {}; } return m_followup; }

    void begin(shmy::speech::Graph&& graph, GameMode init_mode, const std::string& id);
    void advance(size_t index = 0);
    bool apply_advance();
    Element get_current_element() const;

};

