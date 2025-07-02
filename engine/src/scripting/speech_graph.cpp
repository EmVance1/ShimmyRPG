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

bool operator==(const SpeechExitInto& a, const SpeechExitInto& b) {
    return a.script == b.script;
}

bool operator==(const SpeechGoto& a, const SpeechGoto& b) {
    return a.vertex == b.vertex;
}

bool operator==(const SpeechVertex& a, const SpeechVertex& b) {
    return a.speaker == b.speaker &&
           a.lines == b.lines &&
           a.outcome == b.outcome;
}

