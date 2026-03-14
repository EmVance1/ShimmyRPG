#include "pch.h"
#include "gui/text.h"


namespace gui {

Text::Text(const Position& position, const Sizing& sizing, const Style& style, const std::string& label)
    : TextWidget(position, sizing, style)
{
    set_label(label);
}

}
