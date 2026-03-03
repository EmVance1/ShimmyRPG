#pragma once
#include <SFML/Window/Event.hpp>
#include <variant>


namespace input {

struct EndTurn {};
struct CursorSelect {};
struct CursorMove {};

using Event = std::variant<CursorSelect, CursorMove>;

void set_keymap(uint32_t key, Event event);
Event get_keymap(uint32_t key);

void set_buttonmap(uint32_t button, Event event);
Event get_buttonmap(uint32_t button);

Event map_event(const sf::Event& event);

}

