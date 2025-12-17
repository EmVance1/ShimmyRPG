#pragma once
#include <SFML/Graphics.hpp>
#include "gui/gui.h"
#include "game/dialogue.h"


class Game;
struct Scene;

class CinematicMode {
private:
    Game* p_game = nullptr;

    Scene& get_scene();

public:
    std::shared_ptr<gui::Panel> dia_gui;
    Dialogue dialogue;

public:
    CinematicMode() = default;

    void init(Game* _game) { p_game = _game; }

    void handle_event(const sf::Event& event);
    void update();
};

