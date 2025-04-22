#pragma once
#include <SFML/Graphics.hpp>


struct Area;

class NormalMode {
private:
    Area* p_area = nullptr;

    void move_to_action(const std::string& target);
    void speak_action(const std::string& target, const std::string& speech);

public:
    NormalMode() = default;

    void init(Area* area) { p_area = area; }

    void handle_event(const sf::Event& event);
    void update();
};

