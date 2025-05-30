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

sf::Vector2f SortBoundary::get_center_of_mass() const {
    if (is_point) {
        return left;
    } else {
        return left + (right - left) * 0.5f;
    }
}


Entity::Entity(
        const std::string& id,
        const sfu::TextureAtlas& texture,
        const sfu::TextureAtlas& outline,
        const sfu::AlphaMap& bitmap,
        const SpatialGraph2d* pathfinder,
        float pathscale, bool is_character
    ) :
    m_id(id),
    // p_texture(&texture),
    // p_outline(&outline),
    m_bitmap(bitmap),
    m_sprite(texture),
    m_outline_sprite(outline),
    m_tracker(pathfinder, pathscale),
    m_is_character(is_character)
{
    if (m_is_character) {
        m_sprite.setOrigin(sf::Vector2f((float)texture.getCellSize().x * 0.5f, (float)texture.getCellSize().y - 10.f));
        m_outline_sprite.setOrigin(m_sprite.getOrigin());
        m_boundary.is_point = true;
        m_collider.radius = 10.f;
    } else {
        m_boundary.is_point = false;
    }
    m_outline_sprite.setColor(sf::Color::Red);
}


const std::string& Entity::get_id() const {
    return m_id;
}

const sfu::AnimatedSprite& Entity::get_sprite() const {
    return m_sprite;
}

const sfu::AnimatedSprite& Entity::get_outline_sprite() const {
    return m_outline_sprite;
}

SortBoundary Entity::get_boundary() const {
    return {
        m_boundary.left + m_sprite.getPosition(),
        m_boundary.right + m_sprite.getPosition(),
        m_boundary.is_point
    };
}

sf::FloatRect Entity::get_AABB() const {
    return { m_sprite.getPosition() - m_sprite.getOrigin(), (sf::Vector2f)m_sprite.getTexture().getSize() };
}

sfu::FloatCircle Entity::get_trigger_collider() const {
    return m_collider;
}


void Entity::set_sprite_position(const sf::Vector2f& position) {
    m_sprite.setPosition(position);
    m_outline_sprite.setPosition(position);
}

void Entity::set_position(const sf::Vector2f& position, const sf::Transform& cart_to_iso) {
    m_tracker.set_position_world(position);
    m_sprite.setPosition(cart_to_iso.transformPoint(position));
    m_outline_sprite.setPosition(m_sprite.getPosition());
    m_collider.position = position;
}

void Entity::set_animation(size_t index) {
    m_sprite.setRow((uint32_t)index);
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


void Entity::set_dialogue(const std::string& filename) {
    m_dialogue_file = filename;
}

const std::string& Entity::get_dialogue() const {
    return m_dialogue_file;
}


bool Entity::contains(const sf::Vector2f& point) const {
    if (!get_AABB().contains(point)) {
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
    m_is_hovered = hovered;
    if (hovered) {
        // m_sprite.setAnimation(*p_outline);
    } else {
        // m_sprite.setAnimation(*p_texture);
    }
}


void Entity::update_motion(const sf::Transform& cart_to_iso) {
    if (m_is_character) {
        m_tracker.progress();
        m_sprite.setPosition(cart_to_iso.transformPoint(m_tracker.get_position_world()));
        m_outline_sprite.setPosition(m_sprite.getPosition());
        m_collider.position = m_tracker.get_position_world();
    }
}

