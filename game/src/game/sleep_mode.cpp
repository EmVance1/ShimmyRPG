#include "pch.h"
#include "normal_mode.h"
#include "world/area.h"
#include "scripts/lua_script.h"


void SleepMode::handle_event(const sf::Event&) {}

void SleepMode::update() {
    for (auto& [_, e] : p_area->entities) {
        e.update(p_area->cart_to_iso);
    }
    for (auto& s : p_area->scripts) {
        s.update();
    }
}

