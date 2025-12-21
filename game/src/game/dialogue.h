#pragma once
#include <SFML/Graphics.hpp>
#include "scripting/speech/graph.h"


enum class GameMode;

class Dialogue {
public:
    enum class State {
        Empty,
        Player,
        Lines,
    };
    struct Line {
        const std::string* speaker;
        const std::string* line;
    };
    using Selection = std::vector<shmy::speech::Edge>;
    using Element = std::variant<Line, Selection>;

private:
    std::string m_id;
    GameMode m_init_mode;

    shmy::speech::Graph m_graph;
    shmy::speech::Line m_line;
    shmy::speech::Vertex m_vert;

    State m_state = State::Empty;
    bool  m_applied = true;
    bool  m_hashook = false;
    std::string m_sethook = "";

public:
    Dialogue() = default;

    bool is_playing() const { return m_state != State::Empty; }
    State get_state() const { return m_state; }
    GameMode get_init_mode() const { return m_init_mode; }
    std::optional<std::string> take_hook() { if (m_hashook) { m_hashook = false; return m_sethook; } else { return {}; } }

    void begin(shmy::speech::Graph&& graph, GameMode init_mode, const std::string& id);
    void advance(shmy::speech::Edge resp = {});
    bool apply_advance();
    Element get_current_element() const;

};

