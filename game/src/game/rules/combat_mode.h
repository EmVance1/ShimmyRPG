#pragma once
#include <SFML/Graphics.hpp>
#include "gui/gui.h"


class Game;
struct Scene;
class Entity;

enum class CombatFaction {
    Ally,
    Enemy,
    EnemysEnemy,
    UnalignedHostile,
};

enum class CombatOutcome {
    Victory,
    ConditionalVictory,
    Stalemate,
    ConditionalDefeat,
    Defeat,
    TPK,
};

struct CombatParticipant {
    std::string id;
    uint32_t initiative;
    CombatFaction faction;
};


class CombatMode {
private:
    Game* p_game = nullptr;

public:
    std::vector<CombatParticipant> participants;
    size_t active_turn = 0;
    bool advance_turn = false;
    float ai_timer = 1.f;
    CombatOutcome outcome = CombatOutcome::Defeat;
    std::shared_ptr<gui::Panel> atk_gui;

private:
    Scene& get_scene();

    const Entity& get_active() const;
    Entity& get_active();
    bool active_is_playable() const;
    void update_ai();

public:
    CombatMode() = default;

    void init(Game* _game) { p_game = _game; }

    void handle_event(const sf::Event& event);
    void update();
};

