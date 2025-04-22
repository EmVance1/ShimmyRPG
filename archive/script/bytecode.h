#pragma once
#include <SFML/Graphics.hpp>
#include <variant>
#include <string>


struct SetFlag      { std::string flag; uint32_t value; };
struct Lock         { std::string entity; };
struct Unlock       { std::string entity; };
struct Pause        { float seconds; };
struct Wait         { float seconds; };
struct Freeze       { std::string entity; float seconds; };
struct SetPath      { std::string entity; sf::Vector2f position; };
struct SetAnimation { std::string entity; uint32_t animation; };
struct SetVoicebank { std::string entity; uint32_t voicebank; };
struct StartDialogue{ std::string filename; };
struct StartScript  { std::string filename; };

#define VARIANT_WITH_ENUM(name, ...) std::variant<__VA_ARGS__>; enum class Operation { __VA_ARGS__ }

using Instruction = VARIANT_WITH_ENUM(Operation,
    SetFlag,
    Lock,
    Unlock,
    Pause,
    Wait,
    Freeze,
    SetPath,
    SetAnimation,
    SetVoicebank,
    StartDialogue,
    StartScript
);

Instruction read_instruction(std::istream& stream);
void write_instruction(std::ostream& stream, const Instruction& instr);

