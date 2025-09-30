#include "pch.h"
#include "gui/text.h"


namespace gui {

Text::Text(const Position& position, const sf::Vector2f& size, const Style& style, const std::string& label)
    : TextWidget(position, size, style)
{
    set_label(label);
}

}
