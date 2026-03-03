#pragma once
#include <SFML/Graphics.hpp>
#include "gui/gui.h"
#include "game/event.h"


class Game;
class Entity;
struct Portal;

namespace shmy::sim {

struct ShowCtxMenu     { uint32_t entity; sf::Vector2f clickpos; };
struct HideCtxMenu     {};

struct SetPathAction   { uint32_t entity; sf::Vector2f target; };
struct MoveToAction    { uint32_t entity; uint32_t target; };
struct SpeakAction     { uint32_t entity; uint32_t target; };
struct ExamineAction   { uint32_t entity; uint32_t target; };
struct UsePortalAction { uint32_t entity; const Portal* target; };

using Event = std::variant<ShowCtxMenu, HideCtxMenu, SetPathAction, MoveToAction, SpeakAction, ExamineAction, UsePortalAction>;


class Simulation {
private:
    Game* p_game = nullptr;
    std::shared_ptr<gui::Panel> root;
    gui::Text* tooltip;
    gui::VerticalList* ctx_menu;
    sf::Vector2f m_oldmap;

    Entity& entity(uint32_t handle);

    void set_path_action(const SetPathAction& action);
    void move_to_action(const MoveToAction& action);
    void speak_action(const SpeakAction& action);
    void examine_action(const ExamineAction& action);
    void use_portal_action(const UsePortalAction& action);

public:
    void init(Game* _game);
    void show();
    void hide();

    void signal_action(const Event& event);
    void handle_input(const sf::Event& event);
    void handle_event(const shmy::Event& event);
    void update();
};

}
