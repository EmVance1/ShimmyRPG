#pragma once
#include <unordered_map>
#include <vector>
#include "objects/entity.h"


std::vector<Entity*> sprites_topo_sort(std::unordered_map<std::string, Entity>& entities);

Entity* top_contains(const std::vector<Entity*>& sorted_entities, const sf::Vector2f& point);

