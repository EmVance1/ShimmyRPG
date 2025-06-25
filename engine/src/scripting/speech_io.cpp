#include "pch.h"
#include "speech_graph.h"
#include "lexer.h"
#include "util/str.h"


SpeechGraph parse_speechgraph(Lexer lexer);

SpeechGraph dialogue_from_file(const std::string& filename) {
    const auto src = read_to_string(filename);
    auto lexer = Lexer(src);
    try {
        return parse_speechgraph(lexer);
    } catch (const std::exception& e) {
        std::cout << e.what() << "\n";
        return {};
    }
}

SpeechGraph dialogue_from_line(const std::string& speaker, const std::string& line) {
    auto res = SpeechGraph();
    res.emplace("entry0", SpeechVertex{
        FlagExpr::True(),
        speaker,
        { line },
        SpeechExit{}
    });
    return res;
}


/*
void write(std::ostream& stream, const std::string& str) {
    const size_t str_size = str.size();
    stream.write((char*)&str_size, sizeof(size_t));
    stream.write(str.data(), str_size);
}


template<typename T>
concept Writable = requires (std::ostream& stream, T a) {
    { write(stream, a) };
};

template<typename T>
concept Primitive = std::is_integral_v<T> || std::is_floating_point_v<T>;


template<Primitive T>
void write(std::ostream& stream, const std::vector<T>& vec) {
    const size_t vec_size = vec.size();
    stream.write((char*)&vec_size, sizeof(size_t));
    for (const auto& e : vec) {
        stream.write((char*)&e, sizeof(e));
    }
}

template<Writable T>
void write(std::ostream& stream, const std::vector<T>& vec) {
    const size_t vec_size = vec.size();
    stream.write((char*)&vec_size, sizeof(size_t));
    for (const auto& e : vec) {
        write(stream, e);
    }
}

void write(std::ostream& stream, const FlagExpr& condition) {
}

void write(std::ostream& stream, const SpeechResponse& response) {
    write(stream, response.conditions);
    write(stream, response.text);
    write(stream, response.edge);
    write(stream, response.set_flags);
    std::unordered_map<std::string, FlagMod> set_flags;
}

void write(std::ostream& stream, const SpeechOutcome& outcome) {
    const size_t index = outcome.index();
    stream.write((char*)&index, sizeof(size_t));
    if (index == 0) {
        const auto& vec = std::get<std::vector<SpeechResponse>>(outcome);
        write(stream, vec);
    } else if (index == 2) {
        const auto& edge = std::get<SpeechGoto>(outcome);
        write(stream, edge.vertex);
    }
}

void write(std::ostream& stream, const SpeechVertex& point) {
    write(stream, point.speaker);
    write(stream, point.lines);
    SpeechOutcome responses;
}

void write(std::ostream& stream, const SpeechGraph& graph) {
    const size_t count = graph.size();
    stream.write((char*)&count, sizeof(size_t));
    for (const auto& [k, v] : graph) {
        write(stream, k);
        write(stream, v);
    }
}
*/

