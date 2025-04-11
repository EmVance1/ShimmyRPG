#include "mscmptest/asserts.h"
#include "script/speech_graph.h"
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
    const auto testcase = "^02 ShimmyPrime \"Well... Since yez ask...\", \"Got here my special brew. Gots to be strong enough for it though.\", ^brian";

    const auto res = point_from_string(testcase);

    const auto key = std::string("02");
    const auto val = SpeechPoint{
        std::string("ShimmyPrime"),
        std::vector<SpeechLine>{
            SpeechLine{
                false,
                std::string("Well... Since yez ask..."),
            },
            SpeechLine{
                false,
                std::string("Got here my special brew. Gots to be strong enough for it though."),
            },
            SpeechLine{
                true,
                std::string("brian"),
            }
        },
        {}
    };

    assert_eq(key, res.first);
    assert_eq(val, res.second);
}

void parse_vertex() {
    const auto testcase = "^02 ShimmyPrime \"Well... Since yez ask...\", \"Got here my special brew. Gots to be strong enough for it though.\", ^brian; \"Fine. Whisky.\" > 10 > [ PlayerAlcohol: +2, Shimmy>Approval: +2 ], \"I said water.\" > 11, ?(PlayerOriginTwinvayne) \"[Twinvayne Local] I know better than to trust your drinks, Shimmy.\" > 12; ";

    const auto res = speech_from_string(testcase);

    auto val = SpeechGraph();
    val["02"] = SpeechPoint{
        std::string("ShimmyPrime"),
        std::vector<SpeechLine>{
            SpeechLine{
                false,
                std::string("Well... Since yez ask..."),
            },
            SpeechLine{
                false,
                std::string("Got here my special brew. Gots to be strong enough for it though."),
            },
            SpeechLine{
                true,
                std::string("brian"),
            }
        },
        SpeechOutcome{ std::vector<SpeechResponse>{
                SpeechResponse{ {}, "Fine. Whisky.", "10", { { "PlayerAlcohol", FlagInc{ 2 } }, { "Shimmy>Approval", FlagInc{ 2 } } } },
                SpeechResponse{ {}, "I said water.", "11", {} },
                SpeechResponse{ { "PlayerOriginTwinvayne" }, "[Twinvayne Local] I know better than to trust your drinks, Shimmy.", "12", {} },
            }
        }
    };

    assert_eq(val, res);
}

