#include "pch.h"
#include "gui/image.h"


namespace gui {

Image::Image(const Position& position, const sf::Vector2f& size, const Style& style, const sfu::TextureAtlas& texture)
    : Widget(position, size, style)
{
    // m_background.setFillTexture(texture);
}

}
