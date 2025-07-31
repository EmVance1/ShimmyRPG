#define TEST_ROOT
#include "mscmptest/asserts.h"
#include "scripting/speech_graph.h"
#include "scripting/lexer.h"


SpeechGraph parse_speechgraph(Lexer lexer);

test(parse_vertex) {
    const auto testcase = std::string("v02 = ShimmyPrime: [ \"Well... Since yez ask...\", \"Got here my special brew. Gots to be strong enough for it though.\" ] => { \"Fine. Whisky.\" => v10{ PlayerAlcohol: Add(2), Shimmy_Approval: Add(2) }, \"I said water.\" => v11{}, ?(PlayerOriginTwinvayne) \"[Twinvayne Local] I know better than to trust your drinks, Shimmy.\" => v12{} }");

    auto lexer = Lexer(testcase);
    const auto res = parse_speechgraph(lexer);

    auto val = SpeechGraph();
    val["v02"] = SpeechVertex{
        FlagExpr::True(),
        std::string("ShimmyPrime"),
        std::vector<std::string>{
            std::string("Well... Since yez ask..."),
            std::string("Got here my special brew. Gots to be strong enough for it though."),
        },
        SpeechOutcome{ std::vector<SpeechResponse>{
                SpeechResponse{ {}, "Fine. Whisky.", "v10", { { "PlayerAlcohol", FlagAdd{ 2, true } }, { "Shimmy_Approval", FlagAdd{ 2, true } } } },
                SpeechResponse{ {}, "I said water.", "v11", {} },
                SpeechResponse{ FlagExpr::Gt(FlagExpr::Identifier("PlayerOriginTwinvayne"), FlagExpr::Value(0)),
                                    "[Twinvayne Local] I know better than to trust your drinks, Shimmy.", "v12", {} },
            }
        }
    };

    assert_eq(val, res);
}

