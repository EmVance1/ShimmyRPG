#pragma once
#include "normal_mode.h"
#include "cinematic_mode.h"
#include "combat_mode.h"


enum class GameMode {
    Normal = 0,
    Dialogue,
    Cinematic,
    Combat,
};

