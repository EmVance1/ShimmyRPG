#include "pch.h"
#include "speech_graph.h"


bool operator==(const SpeechResponse& a, const SpeechResponse& b) {
    return a.conditions == b.conditions &&
           a.text == b.text &&
           a.edge == b.edge &&
           a.flags == b.flags;
}

bool operator==(const SpeechExit&, const SpeechExit&) {
    return true;
}

bool operator==(const SpeechGoto& a, const SpeechGoto& b) {
    return a.vertex == b.vertex;
}

bool operator==(const SpeechVertex& a, const SpeechVertex& b) {
    return a.speaker == b.speaker &&
           a.lines == b.lines &&
           a.outcome == b.outcome;
}


std::ostream& operator<<(std::ostream& stream, const SpeechResponse& p) {
    return stream << "{ conditions: " << p.conditions << ", text: \"" << p.text << "\", edge: \"" << p.edge << "\", flags: " << p.flags << " }";
}

std::ostream& operator<<(std::ostream& stream, const SpeechOutcome& p) {
    if (const auto resps = std::get_if<std::vector<SpeechResponse>>(&p)) {
        return stream << *resps;
    } else if (const auto go = std::get_if<SpeechGoto>(&p)) {
        return stream << go->vertex;
    } else {
        return stream << "exit";
    }
}

std::ostream& operator<<(std::ostream& stream, const SpeechVertex& p) {
    return stream << "{ speaker: \"" << p.speaker << "\", lines: " << p.lines << ", outcome: " << p.outcome << " }";
}

