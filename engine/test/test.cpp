#define TEST_ROOT
#include "vangotest/asserts.h"
#include "scripting/speech/graph.h"
#include "scripting/lexer.h"
using namespace shmy;


speech::Graph parse_speechgraph(detail::Lexer lexer);


test(parse_vertex) {
    const auto testcase = std::string("v02 = ShimmyPrime: [ \"Well... Since yez ask...\", \"Got here my special brew. Gots to be strong enough for it though.\" ] => { \"Fine. Whisky.\" => v10{ PlayerAlcohol: Add(2), Shimmy_Approval: Add(2) }, \"I said water.\" => v11{}, ?(PlayerOriginTwinvayne) \"[Twinvayne Local] I know better than to trust your drinks, Shimmy.\" => v12{} }");

    auto lexer = detail::Lexer(testcase);
    const auto res = parse_speechgraph(lexer);

    auto val = speech::Graph();
    val["v02"] = speech::Vertex{
        FlagExpr::True(),
        std::string("ShimmyPrime"),
        std::vector<std::string>{
            std::string("Well... Since yez ask..."),
            std::string("Got here my special brew. Gots to be strong enough for it though."),
        },
        speech::Outcome{ std::vector<speech::Response>{
                speech::Response{ {}, "Fine. Whisky.", "v10", { { "PlayerAlcohol", FlagAdd{ 2, true } }, { "Shimmy_Approval", FlagAdd{ 2, true } } } },
                speech::Response{ {}, "I said water.", "v11", {} },
                speech::Response{ FlagExpr::Gt(FlagExpr::Identifier("PlayerOriginTwinvayne"), FlagExpr::Number(0)),
                                    "[Twinvayne Local] I know better than to trust your drinks, Shimmy.", "v12", {} },
            }
        }
    };

    assert_eq(val, res);
}

