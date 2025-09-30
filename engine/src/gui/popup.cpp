#include "pch.h"
#include "gui/popup.h"
#include "gui/text.h"
#include "gui/button.h"


namespace gui {

Popup::Popup(const Position& position, const sf::Vector2f& size, const Style& style, const std::string& label)
    : Container(position, size, style)
{
    auto label_ui = Text::create(Position::topleft({0, 0}), size, style, label);
    label_ui->set_text_alignment(Position::Alignment::Center);
    label_ui->set_background_texture(sf::IntRect{ { 0, 0 }, { 10, 10 } });
    add_widget(label_ui);

    auto close_ui = Button::create(Position::topright({0, 0}), sf::Vector2f(30, 30), style, "");
    close_ui->set_text_alignment(Position::Alignment::Center);
    close_ui->set_callback([&](){ this->destroy(); });
    close_ui->set_sorting_layer(1);
    close_ui->set_background_texture(sf::IntRect{ { 160, 110 }, { 30, 30 } });
    add_widget(close_ui);

    set_sorting_layer(10);
}

}
