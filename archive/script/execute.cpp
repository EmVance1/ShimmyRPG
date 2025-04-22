#include "pch.h"
#include "execute.h"
#include "bytecode.h"
#include "time/deltatime.h"
#include "util/str.h"
#include <fstream>
#include "entity.h"
#include "scripting/lexer.h"
#include "parser.h"
#include "flags.h"


Entity& Script::lookup_entity(const std::string& id) {
    return p_entities->at(p_entity_lookup->at(id));
}

const Entity& Script::lookup_entity(const std::string& id) const {
    return p_entities->at(p_entity_lookup->at(id));
}


Script::Script(std::unordered_map<std::string, Entity>& entities, std::unordered_map<std::string, std::string>& entity_lookup)
    : p_entities(&entities), p_entity_lookup(&entity_lookup)
{}

Script::Script(std::unordered_map<std::string, Entity>& entities, std::unordered_map<std::string, std::string>& entity_lookup, const std::string& filename, bool autoplay)
    : m_external_pause(!autoplay), p_entities(&entities), p_entity_lookup(&entity_lookup)
{
    load_binary(filename);
}


void Script::load_source(const std::string& filename, bool autoplay) {
    m_script.clear();
    m_script_index = 0;
    const auto src = read_to_string(filename);
    auto lexer = Lexer(src);
    m_script = parse_script(lexer);
    m_external_pause = !autoplay;
}

void Script::load_binary(const std::string& filename, bool autoplay) {
    m_script.clear();
    m_script_index = 0;
    auto f = std::ifstream(filename, std::ios::binary);
    while (f.eof()) {
        m_script.push_back(read_instruction(f));
    }
    m_external_pause = !autoplay;
}


void Script::update() {
    while (m_wait <= 0.f && m_pause <= 0.f && m_script_index < m_script.size() && !m_external_pause) {
        const auto next = m_script[m_script_index++];

        switch ((Operation)next.index()) {
        case Operation::SetFlag: {
            const auto v = std::get<SetFlag>(next);
            FlagTable::set_flag(v.flag, v.value);
            break; }
        case Operation::Lock: {
            const auto l = std::get<Lock>(next);
            lookup_entity(l.entity).lock_tracker();
            break; }
        case Operation::Unlock: {
            const auto l = std::get<Unlock>(next);
            lookup_entity(l.entity).unlock_tracker();
            break; }
        case Operation::Pause: {
            const auto p = std::get<Pause>(next);
            m_pause = p.seconds;
            break; }
        case Operation::Wait: {
            const auto w = std::get<Wait>(next);
            m_pause = w.seconds;
            break; }
        case Operation::Freeze: {
            const auto f = std::get<Freeze>(next);
            m_freezes[f.entity] = f.seconds;
            break; }
        case Operation::SetPath: {
            const auto p = std::get<SetPath>(next);
            lookup_entity(p.entity).set_tracker_target(p.position);
            break; }
        case Operation::SetAnimation: {
            const auto a = std::get<SetAnimation>(next);
            lookup_entity(a.entity).set_animation(a.animation);
            break; }
        case Operation::SetVoicebank: {
            const auto v = std::get<SetVoicebank>(next);
            // lookup_entity(v.entity).set_voicebank(a.voicebank);
            break; }
        case Operation::StartDialogue: {
            const auto d = std::get<StartDialogue>(next);
            // lookup_entity(d.entity).set_voicebank(a.voicebank);
            break; }
        case Operation::StartScript: {
            const auto s = std::get<StartScript>(next);

            break; }
        }
    }

    m_wait -= Time::deltatime();
    m_pause -= Time::deltatime();
    for (auto& [_, t] : m_freezes) {
        t -= Time::deltatime();
    }
}

