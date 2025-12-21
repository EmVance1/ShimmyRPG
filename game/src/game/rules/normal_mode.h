#pragma once
#include <SFML/Graphics.hpp>
#include "gui/panel.h"
#include "game/action.h"


class Game;
struct Scene;

class NormalMode {
private:
    Game* p_game = nullptr;
    gui::Panel* gui;

    Scene& get_scene();

    void move_to_action(const MoveToAction& action);
    void speak_action(const SpeakAction& action);
    void examine_action(const ExamineAction& action);

public:
    NormalMode() = default;

    void init(Game* _game);

    void handle_event(const sf::Event& event);
    void update();
};

