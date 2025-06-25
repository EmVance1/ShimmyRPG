#pragma once
#include <unordered_map>
#include <variant>
#include <string>
#include "flags.h"
#include "flag_expr.h"


struct SpeechResponse {
    FlagExpr conditions;
    std::string text;
    std::string edge;
    std::unordered_map<std::string, FlagModifier> flags;
};
struct SpeechGoto { std::string vertex; };
struct SpeechExit {};

using SpeechOutcome = std::variant<std::vector<SpeechResponse>, SpeechGoto, SpeechExit>;

struct SpeechVertex {
    FlagExpr conditions;
    std::string speaker;
    std::vector<std::string> lines;
    SpeechOutcome outcome;
};

using SpeechGraph = std::unordered_map<std::string, SpeechVertex>;


SpeechGraph dialogue_from_file(const std::string& filename);
SpeechGraph dialogue_from_string(const std::string& str);
SpeechGraph dialogue_from_line(const std::string& speaker, const std::string& line);


bool operator==(const SpeechResponse& a, const SpeechResponse& b);
bool operator==(const SpeechExit& a, const SpeechExit& b);
bool operator==(const SpeechGoto& a, const SpeechGoto& b);
bool operator==(const SpeechVertex& a, const SpeechVertex& b);

