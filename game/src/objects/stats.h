#pragma once
#include <cstdint>


struct AbilityScores {
    int strength     = 10;
    int agility      = 10;
    int stamina      = 10;
    int constitution = 10;
    int knowledge    = 10;
    int insight      = 10;
    int charisma     = 10;
    int presence     = 10;
};

struct EntityStats {
    uint32_t level = 1;

    float movement;

    AbilityScores scores;
    AbilityScores scores_current;

    uint32_t max_hp() const;
    float max_movement() const;

    void reset_turn();
};

