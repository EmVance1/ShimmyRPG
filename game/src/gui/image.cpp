#include "pch.h"
#include "image.h"


namespace gui {

Image::Image(const Position& position, const sf::Vector2f& size, const Style& style, const sf::Texture& texture)
    : Widget(position, size, style)
{
    set_background_texture(texture);
}

}
