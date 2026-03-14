#include "pch.h"
#include "gui/window.h"
#include "gui/button.h"


namespace gui {

Window::Window(const Position& position, const Sizing& sizing, const Style& style)
    : Container(position, sizing, style)
{
    auto close_ui = Button::create(Position{ lo::right(0), lo::top(0) }, Sizing({30, 30}), style, "");
    close_ui->set_text_position(lo::center({0, 0}));
    close_ui->set_callback([&](){ this->destroy(); });
    close_ui->set_sorting_layer(10000);
    close_ui->set_background_texture(sf::IntRect{ { 150, 100 }, { 30, 30 } });
    add_widget(close_ui);
}

}
