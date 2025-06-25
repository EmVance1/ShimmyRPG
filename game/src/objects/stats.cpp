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
    return scores.strength * 1 + scores.constitution * 2 + scores.stamina * 3 + level * 10;
}

float EntityStats::max_movement() const {
    // ca 300 + 500 + 40..400
    return (float)(scores.agility * 30 + scores.stamina * 50 + level * 40);
}


void EntityStats::reset_turn() {
    movement = max_movement();
}

