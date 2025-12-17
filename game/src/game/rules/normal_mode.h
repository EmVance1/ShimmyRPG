#pragma once
#include <SFML/Graphics.hpp>
#include "gui/panel.h"


class Game;
struct Scene;

class NormalMode {
private:
    Game* p_game = nullptr;
    gui::Panel* gui;

    Scene& get_scene();

    void move_to_action(const std::string& target);
    void speak_action(const std::string& target, const std::string& speech);

public:
    NormalMode() = default;

    void init(Game* _game);

    void handle_event(const sf::Event& event);
    void update();
};

