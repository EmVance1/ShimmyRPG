#include "pch.h"
#include "normal_mode.h"
#include "world/area.h"
#include "scripting/lua/script.h"


void SleepMode::handle_event(const sf::Event&) {}

void SleepMode::update() {
    for (auto& [_, e] : p_area->entities) {
        e.update(p_area->cart_to_iso);
    }
    p_area->lua_vm.update();
}

