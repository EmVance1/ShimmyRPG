#include "pch.h"
#include "image.h"


namespace gui {

Image::Image(const Position& position, const sf::Vector2f& size, const Style& style, const sf::Texture& texture)
    : Widget(position, size, style)
{
    m_shape.setSize(size);
    m_shape.setTexture(&texture);
}


void Image::draw(sf::RenderTarget& target, sf::RenderStates states) const {
    states.transform *= get_transform();
    target.draw(m_shape, states);
}

}
