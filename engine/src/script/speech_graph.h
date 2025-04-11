#pragma once
#include <unordered_map>
#include <variant>
#include <string>
#include "flags.h"


struct SpeechResponse {
    std::vector<std::string> conditions;
    std::string text;
    std::string goes_to;
    std::unordered_map<std::string, FlagMod> set_flags;
};

struct SpeechEnd {};

using SpeechOutcome = std::variant<std::vector<SpeechResponse>, SpeechEnd>;

struct SpeechLine {
    bool is_goto;
    std::string text;
};

struct SpeechPoint {
    std::string speaker_id;
    std::vector<SpeechLine> lines;
    SpeechOutcome responses;
};

using SpeechGraph = std::unordered_map<std::string, SpeechPoint>;


using SpeechPointTuple = std::pair<std::string, SpeechPoint>;

SpeechGraph speech_from_string(const std::string& str);
SpeechPointTuple point_from_string(const std::string& str);
std::vector<std::string> conditions_from_string(const std::string& str);
std::unordered_map<std::string, FlagMod> setflags_from_string(const std::string& str);
SpeechResponse option_from_string(const std::string& str);
SpeechOutcome outcome_from_string(const std::string& str);
SpeechGraph speech_from_string(const std::string& str);

bool operator==(const SpeechLine& a, const SpeechLine& b);
bool operator==(const SpeechResponse& a, const SpeechResponse& b);
bool operator==(const SpeechEnd& a, const SpeechEnd& b);
bool operator==(const SpeechPoint& a, const SpeechPoint& b);

std::ostream& operator<<(std::ostream& stream, const SpeechPoint& p);
std::ostream& operator<<(std::ostream& stream, const SpeechOutcome& p);
std::ostream& operator<<(std::ostream& stream, const SpeechResponse& p);
std::ostream& operator<<(std::ostream& stream, const SpeechGraph& p);

