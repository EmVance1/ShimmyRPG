#pragma once
#include <SFML/Graphics.hpp>
#include "bases/textwidget.h"


namespace gui {

class Text : public TextWidget {
public:
    Text(const Position& position, const Sizing& sizing, const Style& style, const std::string& label);

    static std::shared_ptr<Text> create(const Position& position, const Sizing& sizing, const Style& style, const std::string& label) {
        return std::make_shared<Text>(position, sizing, style, label);
    }
};

}
