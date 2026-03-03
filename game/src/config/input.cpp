#include "pch.h"
#include "input.h"
#include <unordered_map>


namespace input {

static std::unordered_map<uint32_t, Event> keymaps;
static std::unordered_map<uint32_t, Event> buttonmaps;
enum class Scheme {
    Keyboard,
    Controller,
};
static Scheme active_scheme;

void set_keymap(uint32_t key, Event event) {
    keymaps[key] = event;
}
Event get_keymap(uint32_t key) {
    return keymaps[key];
}

void set_buttonmap(uint32_t button, Event event) {
    buttonmaps[button] = event;
}
Event get_buttonmap(uint32_t button) {
    return buttonmaps[button];
}

Event map_event(const sf::Event& event) {
    return CursorSelect{};
}

}
