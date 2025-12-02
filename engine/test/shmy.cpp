#define _CRT_SECURE_NO_WARNINGS
#define VANGO_TEST_ROOT
#include <vangotest/asserts2.h>
#include <vangotest/bench.h>
#include "scripting/speech/graph.h"
#include "scripting/speech2/graph.h"
#include "core/fs.h"


vango_test(bench_load_shmy1) {
    size_t sink = 0;

    vango_bench(100000, [&](){
        const auto g = shmy::speech::Graph::load_from_file("test/res/BertDialogue.shmy");
        sink += (size_t)g.vertices[0].lines[0][0];
    });

    printf("sink: %llu\n", sink);
}

vango_test(bench_parse_shmy1) {
    const auto _src = shmy::core::fs::read_to_string("test/res/BertDialogue.shmy").unwrap();
    const auto src = std::string(_src.c_str());
    size_t sink = 0;

    vango_bench(100000, [&](){
        const auto g = shmy::speech::Graph::load_from_string(src);
        sink += (size_t)g.vertices[0].lines[0][0];
    });

    printf("sink: %llu\n", sink);
}


vango_test(bench_load_shmy2) {
    size_t sink = 0;

    vango_bench(100000, [&](){
        const auto g = shmy::speech2::Graph::load_from_file("test/res/BertDialogue.shmy");
        sink += (size_t)g.verts[0].n_lines;
    });

    printf("sink: %llu\n", sink);
}

vango_test(bench_parse_shmy2) {
    const auto _src = shmy::core::fs::read_to_string("test/res/BertDialogue.shmy").unwrap();
    const auto src = std::string(_src.c_str());
    size_t sink = 0;

    vango_bench(100000, [&](){
        const auto g = shmy::speech2::Graph::load_from_string(src);
        sink += (size_t)g.verts[0].n_lines;
    });

    printf("sink: %llu\n", sink);
}

