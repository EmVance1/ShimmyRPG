#include "pch.h"
#include "bytecode.h"


Instruction read_instruction(std::istream& stream) {
    Operation op;
    stream.read((char*)&op, sizeof(uint8_t));

    switch (op) {
    case Operation::Pause: {
        auto data = Pause{};
        stream.read((char*)&data.seconds, sizeof(float));
        return data; }
    case Operation::Wait: {
        auto data = Wait{};
        stream.read((char*)&data.seconds, sizeof(float));
        return data; }
    case Operation::Freeze: {
        auto data = Freeze{};
        auto size = (size_t)0;
        stream.read((char*)&size, sizeof(size_t));
        data.entity.reserve(size);
        stream.read(data.entity.data(), size);
        stream.read((char*)&data.seconds, sizeof(float));
        return data; }
    case Operation::SetPath: {
        auto data = SetPath{};
        auto size = (size_t)0;
        stream.read((char*)&size, sizeof(size_t));
        data.entity.reserve(size);
        stream.read(data.entity.data(), size);
        stream.read((char*)&data.position, sizeof(sf::Vector2f));
        return data; }
    case Operation::SetAnimation: {
        auto data = SetAnimation{};
        auto size = (size_t)0;
        stream.read((char*)&size, sizeof(size_t));
        data.entity.reserve(size);
        stream.read(data.entity.data(), size);
        stream.read((char*)&data.animation, sizeof(uint32_t));
        return data; }
    case Operation::SetVoicebank: {
        auto data = SetVoicebank{};
        auto size = (size_t)0;
        stream.read((char*)&size, sizeof(size_t));
        data.entity.reserve(size);
        stream.read(data.entity.data(), size);
        stream.read((char*)&data.voicebank, sizeof(uint32_t));
        return data; }
    case Operation::Dialogue: {
        auto data = Dialogue{};
        return data; }
    }

    return Instruction{};
}

void write_instruction(std::ostream& stream, const Instruction& instr) {
    const uint8_t i = (uint8_t)instr.index();
    stream.write((char*)&i, sizeof(char));

    switch ((Operation)instr.index()) {
    case Operation::Pause: {
        const auto data = std::get<Pause>(instr);
        stream.write((char*)&data.seconds, sizeof(float));
        break; }
    case Operation::Wait: {
        const auto data = std::get<Wait>(instr);
        stream.write((char*)&data.seconds, sizeof(float));
        break; }
    case Operation::Freeze: {
        const auto data = std::get<Freeze>(instr);
        const auto temp = data.entity.size();
        stream.write((char*)&temp, sizeof(size_t));
        stream.write(data.entity.data(), data.entity.size());
        stream.write((char*)&data.seconds, sizeof(float));
        break; }
    case Operation::SetPath: {
        const auto data = std::get<SetPath>(instr);
        const auto temp = data.entity.size();
        stream.write((char*)&temp, sizeof(size_t));
        stream.write((char*)&data.position, sizeof(sf::Vector2f));
        break; }
    case Operation::SetAnimation: {
        const auto data = std::get<SetAnimation>(instr);
        const auto temp = data.entity.size();
        stream.write((char*)&temp, sizeof(size_t));
        stream.write((char*)&data.animation, sizeof(uint32_t));
        break; }
    case Operation::SetVoicebank: {
        const auto data = std::get<SetVoicebank>(instr);
        const auto temp = data.entity.size();
        stream.write((char*)&temp, sizeof(size_t));
        stream.write((char*)&data.voicebank, sizeof(uint32_t));
        break; }
    case Operation::Dialogue: {
        const auto data = std::get<Dialogue>(instr);
        break; }
    }
}

