#include "pch.h"
#include "entity.h"


float SortBoundary::get_threshold(const sf::Vector2f& point) const {
    if (std::abs(right.x - left.x) < 0.0001f) {
        return point.y;
    }
    if (point.x < left.x || point.x > right.x) {
        return point.y;
    }
    const auto dx = right.x - left.x;
    const auto dy = right.y - left.y;
    const auto m = dy / dx;
    const auto q = left.y - (m * left.x);
    return m * point.x + q;
}



Entity::Entity(
        const sfu::AlphaMap& bitmap,
        const sf::Texture& texture,
        const sf::Texture& outline,
        const std::string& id,
        const SpatialGraph2d* pathfinder,
        float pathscale, bool _is_character
    )
    : m_bitmap(bitmap),
    p_texture(&texture),
    p_outline(&outline),
    m_id(id),
    m_sprite(texture),
    m_tracker(pathfinder, pathscale),
    m_is_character(_is_character)
{
    if (m_is_character) {
        m_sprite.setOrigin(sf::Vector2f((float)texture.getSize().x * 0.5f, (float)texture.getSize().y - 10.f));
        m_boundary.is_point = true;
        m_collider.radius = 30.f;
    } else {
        m_boundary.is_point = false;
    }
}


const std::string& Entity::get_id() const {
    return m_id;
}

const sf::Sprite& Entity::get_sprite() const {
    return m_sprite;
}

SortBoundary Entity::get_boundary() const {
    return {
        m_boundary.left + m_sprite.getPosition(),
        m_boundary.right + m_sprite.getPosition(),
        m_boundary.is_point
    };
}

sf::FloatRect Entity::get_AABB() const {
    return m_sprite.getGlobalBounds();
}

sfu::FloatCircle Entity::get_trigger_collider() const {
    return m_collider;
}


void Entity::set_sprite_position(const sf::Vector2f& position) {
    m_sprite.setPosition(position);
}

void Entity::set_position(const sf::Vector2f& position, const sf::Transform& cart_to_iso) {
    m_tracker.set_position_world(position);
    m_sprite.setPosition(cart_to_iso.transformPoint(position));
    m_collider.position = position;
}


void Entity::set_sorting_boundary(const sf::Vector2f& pos) {
    m_boundary.left = pos;
    m_boundary.is_point = true;
}

void Entity::set_sorting_boundary(const sf::Vector2f& left, const sf::Vector2f& right) {
    m_boundary.left = left;
    m_boundary.right = right;
    m_boundary.is_point = false;
}


bool Entity::set_tracker_target(const sf::Vector2f& cartesian) {
    return m_tracker.set_path_world(cartesian);
}

sf::Vector2f Entity::get_tracker_target() const {
    return m_tracker.get_target_position_world();
}

void Entity::set_tracker_speed(float speed) {
    m_tracker.set_speed(speed);
}

bool Entity::is_moving() const {
    return m_tracker.is_moving();
}


bool Entity::contains(const sf::Vector2f& point) const {
    if (!m_sprite.getGlobalBounds().contains(point)) {
        return false;
    }
    const auto inv = m_sprite.getInverseTransform().transformPoint(point);
    if ((int)inv.x < 0 || (int)inv.x >= (int)m_bitmap.getSize().x ||
        (int)inv.y < 0 || (int)inv.y >= (int)m_bitmap.getSize().y) {
        return false;
    }
    return m_bitmap.getPixel(sf::Vector2u(inv));
}

void Entity::set_hovered(bool hovered) {
    if (hovered) {
        m_is_hovered = true;
        m_sprite.setTexture(*p_outline);
    } else {
        m_is_hovered = false;
        m_sprite.setTexture(*p_texture);
    }
}


void Entity::update_motion(const sf::Transform& cart_to_iso) {
    if (m_is_character) {
        m_tracker.progress();
        m_sprite.setPosition(cart_to_iso.transformPoint(m_tracker.get_position_world()));
        m_collider.position = m_tracker.get_position_world();
    }
}

