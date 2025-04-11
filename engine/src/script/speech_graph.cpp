#include "pch.h"
#include "speech_graph.h"
#include "util/str.h"


std::vector<std::string> split_paras(const std::string& str, char delimiter) {
    auto result = std::vector<std::string>();
    size_t last = 0;
    bool dont = false;
    bool instr = false;
    bool inbrac = false;

    for (size_t i = 0; i < str.size(); i++) {
        if (str[i] == '"') {
            instr = !instr;
        }
        if (!instr) {
            if (str[i] == '[') {
                inbrac = true;
            }
            if (str[i] == ']') {
                inbrac = false;
            }
        }

        if (str[i] == delimiter && !instr && !inbrac) {
            result.push_back(str.substr(last, i - last));
            if (i + 1 < str.size()) {
                last = i + 1;
            } else {
                dont = true;
            }
        }
    }

    if (!dont) {
        result.push_back(str.substr(last, str.size() - last));
    }

    return result;
}


SpeechPointTuple point_from_string(const std::string& str) {
    size_t i = 1;
    while (!std::isspace(str[i]) && i < str.size()) {
        i++;
    }
    const auto key = str.substr(1, i - 1);
    while (std::isspace(str[i])) {
        i++;
    }
    const auto start = i;
    while (!std::isspace(str[i]) && i < str.size()) {
        i++;
    }
    const auto speaker = str.substr(start, i - start);

    const auto lines = split_paras(str.substr(i), ',');

    auto res_lines = std::vector<SpeechLine>();

    for (const auto& l : lines) {
        const auto trim = trim_string(l);
        if (trim[0] == '"') {
            res_lines.push_back(SpeechLine{ false, trim.substr(1, trim.size() - 2) });
        } else if (trim[0] == '^') {
            res_lines.push_back(SpeechLine{ true, trim.substr(1) });
        }
    }

    return { key, SpeechPoint{ speaker, res_lines, {} } };
}

std::vector<std::string> conditions_from_string(const std::string& str) {
    return { str.substr(1, str.size() - 2) };
}

std::unordered_map<std::string, FlagMod> setflags_from_string(const std::string& str) {
    const auto temp = str.substr(1, str.size() - 2);
    const auto flags = split_paras(temp, ',');
    auto result = std::unordered_map<std::string, FlagMod>();
    for (const auto& f : flags) {
        const auto split = split_string(f, ':');
        const auto trim = trim_string(split[1]);
        if (trim[0] == '+' || trim[0] == '-') {
            result[trim_string(split[0])] = FlagInc{ std::atoi(trim.c_str()) };
        } else {
            result[trim_string(split[0])] = FlagSet{ std::atoi(trim.c_str()) };
        }
    }
    return result;
}

SpeechResponse option_from_string(const std::string& str) {
    auto result = SpeechResponse();

    size_t shift = 0;
    if (str[0] == '?') {
        for (size_t i = 1; str[i] != '"' && i < str.size(); i++) {
            shift++;
        }
        const auto flag_expr = str.substr(1, shift);
        result.conditions = conditions_from_string(trim_string(flag_expr));
        shift++;
    }

    const auto comps = split_paras(str.substr(shift), '>');

    result.text = trim_string(comps[0]);
    result.text = result.text.substr(1, result.text.size() - 2);
    result.goes_to = trim_string(comps[1]);
    if (comps.size() == 3) {
        result.set_flags = setflags_from_string(trim_string(comps[2]));
    }

    return result;
}

SpeechOutcome outcome_from_string(const std::string& str) {
    if (str == "end") {
        return SpeechEnd();
    }

    const auto options = split_paras(str, ',');

    auto result = std::vector<SpeechResponse>();

    for (const auto& op : options) {
        const auto trim = trim_string(op);
        if (trim.empty()) { continue; }
        result.push_back(option_from_string(trim));
    }

    return result;
}


SpeechGraph speech_from_string(const std::string& str) {
    const auto statements = split_string(str, ';');
    auto list = std::vector<SpeechPointTuple>();

    for (const auto& s : statements) {
        const auto trim = trim_string(s);
        if (trim.empty()) { continue; }
        if (trim[0] == '^') {
            list.push_back(point_from_string(trim));
        } else {
            const auto temp = outcome_from_string(trim);
            list.back().second.responses = temp;
        }
    }

    auto result = SpeechGraph();

    for (const auto& l : list) {
        result[l.first] = l.second;
    }

    return result;
}


bool operator==(const SpeechLine& a, const SpeechLine& b) {
    return a.is_goto == b.is_goto &&
           a.text == b.text;
}

bool operator==(const SpeechResponse& a, const SpeechResponse& b) {
    return a.conditions == b.conditions &&
           a.text == b.text &&
           a.goes_to == b.goes_to &&
           a.set_flags == b.set_flags;
}

bool operator==(const SpeechEnd& a, const SpeechEnd& b) {
    return true;
}

bool operator==(const SpeechPoint& a, const SpeechPoint& b) {
    return a.speaker_id == b.speaker_id &&
           a.lines == b.lines &&
           a.responses == b.responses;
}


std::ostream& operator<<(std::ostream& stream, const SpeechPoint& p) {
    stream << "{ speaker: \"" << p.speaker_id << "\", [ ";
    for (const auto& line : p.lines) {
        if (line.is_goto) {
            stream << "^" << line.text << ", ";
        } else {
            stream << "\"" << line.text << "\", ";
        }
    }
    return stream << " ], " << p.responses << " }";
}

std::ostream& operator<<(std::ostream& stream, const SpeechOutcome& p) {
    if (const auto r = std::get_if<std::vector<SpeechResponse>>(&p)) {
        stream << "[ ";
        for (const auto& line : *r) {
            if (!line.conditions.empty()) {
                stream << "[ " << line.conditions[0] << " ] ";
            }
            stream << "\"" << line.text << "\" \"" << line.goes_to << "\" { ";
            for (const auto& [k, v] : line.set_flags) {
                stream << "\"" << k << "\": " << v << ", ";
            }
            stream << " }, ";
        }
        return stream << " ]";
    } else {
        return stream << "end";
    }
}

std::ostream& operator<<(std::ostream& stream, const SpeechResponse& p) {
    return stream << "\"" << p.text << "\"";
}

std::ostream& operator<<(std::ostream& stream, const SpeechGraph& p) {
    stream << "{ ";
    for (const auto& [k, v] : p) {
        stream << "\"" << k << "\": " << v << ", ";
    }
    return stream << " }";
}

