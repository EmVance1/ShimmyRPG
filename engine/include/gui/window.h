#pragma once
#include <SFML/Graphics.hpp>
#include "style.h"
#include "bases/container.h"
#include <memory>


namespace gui {

class Window : public Container {
public:
    Window(const Position& position, const Sizing& sizing, const Style& style);

    static std::shared_ptr<Window> create(const Position& position, const Sizing& sizing, const Style& style) {
        return std::make_shared<Window>(position, sizing, style);
    }
};

}
