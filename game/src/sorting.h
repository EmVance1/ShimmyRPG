#pragma once
#include <unordered_map>
#include <vector>
#include "entity.h"


std::vector<Entity*> sort_sprites(std::unordered_map<std::string, Entity>& entities);

Entity* top_contains(const std::vector<Entity*>& sorted_entities, const sf::Vector2f& point);

