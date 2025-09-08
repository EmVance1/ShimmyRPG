#include "pch.h"
#include "image.h"


namespace gui {

Image::Image(const Position& position, const sf::Vector2f& size, const Style& style, const sfu::TextureAtlas& texture)
    : Widget(position, size, style)
{
    (void)texture;
    // set_background_texture(texture, TextureFillMode::Stretch);
    set_background_color(sf::Color::White);
}

}
