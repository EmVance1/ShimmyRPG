#pragma once
#include <SFML/Graphics.hpp>
#include <jig/jig.hpp>
#include "gui/gui.h"
#include "game/event.h"


class Game;

namespace shmy::sim {

class Cinematic {
public:
    struct BeginSpeech   { std::string modpath; };
    struct ContinueEvent {};
    struct SelectEvent   { jigProposition prop; };
    using Event = std::variant<BeginSpeech, ContinueEvent, SelectEvent>;

private:
    Game* p_game = nullptr;
    jigFile* m_current_file;
    jigVm m_interpreter;
    std::string m_once_prefix;
    std::shared_ptr<::gui::Panel> root;

public:
    bool manual_exit = false;

    void init(Game* _game);
    void show();
    void hide();

    void signal_action(const Event& event);
    void handle_input(const sf::Event& event);
    void handle_event(const shmy::Event& event);
    void update();
};

}
