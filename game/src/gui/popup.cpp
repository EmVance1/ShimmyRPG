#include "pch.h"
#include "popup.h"
#include "text.h"
#include "button.h"


namespace gui {

Popup::Popup(const Position& position, const sf::Vector2f& size, const Style& style, const std::string& label)
    : Container(position, size, style)
{
    auto label_ui = Text::create(Position::topleft({0, 0}), size, style, label);
    label_ui->set_text_alignment(Alignment::Center);
    add_widget(label_ui);

    auto close_ui = Button::create(Position::topright({0, 0}), sf::Vector2f(30, 30), style, "X");
    close_ui->set_text_alignment(Alignment::Center);
    close_ui->set_callback([&](){ this->destroy(); });
    close_ui->set_sorting_layer(1);
    add_widget(close_ui);

    set_sorting_layer(10);
}

}
