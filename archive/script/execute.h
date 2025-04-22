#pragma once
#include "bytecode.h"


class Entity;

class Script {
private:
    std::vector<Instruction> m_script;
    size_t m_script_index = 0;
    std::unordered_map<std::string, float> m_freezes;
    float m_wait = 0.f;
    float m_pause = 0.f;
    bool m_external_pause = false;

    std::unordered_map<std::string, Entity>* p_entities;
    std::unordered_map<std::string, std::string>* p_entity_lookup;

public:
    Script(std::unordered_map<std::string, Entity>& entities, std::unordered_map<std::string, std::string>& entity_lookup);
    Script(std::unordered_map<std::string, Entity>& entities, std::unordered_map<std::string, std::string>& entity_lookup, const std::string& filename, bool autoplay = true);

    Entity& lookup_entity(const std::string& id);
    const Entity& lookup_entity(const std::string& id) const;

    void load_source(const std::string& filename, bool autoplay = true);
    void load_binary(const std::string& filename, bool autoplay = true);

    void restart(bool autoplay = true) { m_script_index = 0; m_external_pause = !autoplay; }
    void pause() { m_external_pause = true; }
    void play() { m_external_pause = false; }

    void update();
};

