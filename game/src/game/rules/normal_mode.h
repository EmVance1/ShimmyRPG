#pragma once
#include <SFML/Graphics.hpp>
#include "gui/panel.h"


class Region;
struct Area;

class NormalMode {
private:
    Region* p_region = nullptr;
    gui::Panel* gui;

    Area& get_area();

    void move_to_action(const std::string& target);
    void speak_action(const std::string& target, const std::string& speech);

public:
    NormalMode() = default;

    void init(Region* _region);

    void handle_event(const sf::Event& event);
    void update();
};

