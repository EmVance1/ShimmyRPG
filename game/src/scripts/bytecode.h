#pragma once
#include <SFML/Graphics.hpp>
#include <variant>
#include <string>
#include "script/speech_graph.h"


enum class Operation {
    Pause,
    Wait,
    Freeze,
    SetPath,
    SetAnimation,
    SetVoicebank,
    Dialogue
};


struct Pause        { float seconds; };
struct Wait         { float seconds; };
struct Freeze       { std::string entity; float seconds; };
struct SetPath      { std::string entity; sf::Vector2f position; };
struct SetAnimation { std::string entity; uint32_t animation; };
struct SetVoicebank { std::string entity; uint32_t voicebank; };
struct Dialogue     { SpeechGraph graph; };

using Instruction = std::variant<Pause, Wait, Freeze, SetPath, SetAnimation, SetVoicebank, Dialogue>;

Instruction read_instruction(std::istream& stream);
void write_instruction(std::ostream& stream, const Instruction& instr);

