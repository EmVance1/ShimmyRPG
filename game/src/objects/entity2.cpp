#include "pch.h"
#include "entity2.h"


namespace shmy {

std::string& Entity::id() { return m_list->id[m_id]; }
const std::string& Entity::id() const { return m_list->id[m_id]; }
std::string& Entity::name() { return m_list->name[m_id]; }
const std::string& Entity::name() const { return m_list->name[m_id]; }
std::unordered_set<std::string>& Entity::tags() { return m_list->tags[m_id]; }
const std::unordered_set<std::string>& Entity::tags() const { return m_list->tags[m_id]; }

sfu::AlphaMap& Entity::bitmap() { return m_list->bitmap[m_id]; }
const sfu::AlphaMap& Entity::bitmap() const { return m_list->bitmap[m_id]; }
sfu::AnimatedSprite& Entity::sprite() { return m_list->sprite[m_id]; }
const sfu::AnimatedSprite& Entity::sprite() const { return m_list->sprite[m_id]; }
sfu::AnimatedSprite& Entity::outline_sprite() { return m_list->outline_sprite[m_id]; }
const sfu::AnimatedSprite& Entity::outline_sprite() const { return m_list->outline_sprite[m_id]; }

nav::Agent& Entity::tracker() { return m_list->tracker[m_id]; }
const nav::Agent& Entity::tracker() const { return m_list->tracker[m_id]; }
SortBoundary& Entity::boundary() { return m_list->boundary[m_id]; }
const SortBoundary& Entity::boundary() const { return m_list->boundary[m_id]; }
sfu::FloatCircle& Entity::collider() { return m_list->collider[m_id]; }
const sfu::FloatCircle& Entity::collider() const { return m_list->collider[m_id]; }

std::string& Entity::dialogue() { return m_list->dialogue[m_id]; }
const std::string& Entity::dialogue() const { return m_list->dialogue[m_id]; }
std::string& Entity::examination() { return m_list->examination[m_id]; }
const std::string& Entity::examination() const { return m_list->examination[m_id]; }
std::vector<Entity::Action>& Entity::actions() { return m_list->actions[m_id]; }
const std::vector<Entity::Action>& Entity::actions() const { return m_list->actions[m_id]; }
std::deque<shmy::sim::Event>& Entity::action_queue() { return m_list->action_queue[m_id]; }
const std::deque<shmy::sim::Event>& Entity::action_queue() const { return m_list->action_queue[m_id]; }

EntityStats& Entity::stats() { return m_list->stats[m_id]; }
const EntityStats& Entity::stats() const { return m_list->stats[m_id]; }
Inventory& Entity::inventory() { return m_list->inventory[m_id]; }
const Inventory& Entity::inventory() const { return m_list->inventory[m_id]; }
uint32_t& Entity::flags() { return m_list->flags[m_id]; }
const uint32_t& Entity::flags() const { return m_list->flags[m_id]; }

bool Entity::is_playable() const { return (m_list->flags[m_id] & Entity::Flag::Playable) > 0; }
bool Entity::is_offstage() const { return (m_list->flags[m_id] & Entity::Flag::Offstage) > 0; }
bool Entity::is_hovered() const  { return (m_list->flags[m_id] & Entity::Flag::Hovered) > 0; }
bool Entity::is_ghost() const    { return (m_list->flags[m_id] & Entity::Flag::Ghost) > 0; }

}
