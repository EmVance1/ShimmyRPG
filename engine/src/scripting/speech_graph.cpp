#include "pch.h"
#include "speech_graph.h"


namespace shmy { namespace speech {

bool operator==(const Vertex& a, const Vertex& b) {
    return a.speaker == b.speaker &&
           a.lines == b.lines &&
           a.outcome == b.outcome;
}

bool operator==(const Response& a, const Response& b) {
    return a.conditions == b.conditions &&
           a.text == b.text &&
           a.edge == b.edge &&
           a.flags == b.flags;
}

bool operator==(const Goto& a, const Goto& b) {
    return a.vertex == b.vertex;
}

bool operator==(const Exit&, const Exit&) {
    return true;
}

bool operator==(const ExitInto& a, const ExitInto& b) {
    return a.script == b.script;
}

} }
