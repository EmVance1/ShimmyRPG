#pragma once
#include "normal_mode.h"
#include "cinematic_mode.h"
#include "combat_mode.h"
#include "sleep_mode.h"


enum class GameMode {
    Normal    = 0,
    Dialogue  = 1,
    Cinematic = 2,
    Combat    = 3,
    Sleep     = 4,
};

