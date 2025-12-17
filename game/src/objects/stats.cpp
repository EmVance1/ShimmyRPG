#include "pch.h"
#include "stats.h"

/*
    int strength;
    int agility;
    int stamina;
    int constitution;
    int knowledge;
    int insight;
    int charisma;
    int presence;
*/


uint32_t EntityStats::max_hp() const {
    // ca 10 + 20 + 30 + 10..100
    return scores.strength * 1u + scores.constitution * 2u + scores.stamina * 3u + level * 10u;
}

float EntityStats::max_movement() const {
    // ca 35 + 45 + 4..40
    const float agility = (float)scores.agility * 3.5f;
    const float stamina = (float)scores.stamina * 4.5f;
    return agility + stamina + (float)level * 4.f;
}


void EntityStats::reset_turn() {
    movement = max_movement();
}

