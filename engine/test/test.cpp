#include "mscmptest/asserts.h"
#include "script/speech_graph.h"
#include "script/speech_lex.h"
#include "str.h"


void parse_point();
void parse_vertex();
void split_commas();
void trim_whitespace();


int main() {
    test(parse_point);
    test(parse_vertex);
    test(split_commas);
    test(trim_whitespace);
}


void parse_point() {
    const auto testcase = std::string("v02(ShimmyPrime): [ \"Well... Since yez ask...\", \"Got here my special brew. Gots to be strong enough for it though.\" ] => brian");

    auto lexer = Lexer(testcase);
    const auto res = parse_vertex(lexer);

    const auto key = std::string("v02");
    const auto val = SpeechVertex{
        std::string("ShimmyPrime"),
        std::vector<std::string>{
            std::string("Well... Since yez ask..."),
            std::string("Got here my special brew. Gots to be strong enough for it though."),
        },
        SpeechGoto{
            std::string("brian"),
        }
    };

    assert_eq(key, res->first);
    assert_eq(val, res->second);
}

void parse_vertex() {
    const auto testcase = std::string("v02(ShimmyPrime): [ \"Well... Since yez ask...\", \"Got here my special brew. Gots to be strong enough for it though.\" ] => { \"Fine. Whisky.\" => v10{ PlayerAlcohol: Add(2), Shimmy_Approval: Add(2) }, \"I said water.\" => v11{}, ?(PlayerOriginTwinvayne) \"[Twinvayne Local] I know better than to trust your drinks, Shimmy.\" => v12{} }");

    auto lexer = Lexer(testcase);
    const auto res = parse_speechgraph(lexer);

    auto val = SpeechGraph();
    val["v02"] = SpeechVertex{
        std::string("ShimmyPrime"),
        std::vector<std::string>{
            std::string("Well... Since yez ask..."),
            std::string("Got here my special brew. Gots to be strong enough for it though."),
        },
        SpeechOutcome{ std::vector<SpeechResponse>{
                SpeechResponse{ {}, "Fine. Whisky.", "v10", { { "PlayerAlcohol", FlagAdd{ 2 } }, { "Shimmy_Approval", FlagAdd{ 2 } } } },
                SpeechResponse{ {}, "I said water.", "v11", {} },
                SpeechResponse{ { FlagCondition{ "PlayerOriginTwinvayne", 0, FlagComparison::IsGreaterThan } },
                                    "[Twinvayne Local] I know better than to trust your drinks, Shimmy.", "v12", {} },
            }
        }
    };

    assert_eq(val, res);
}

