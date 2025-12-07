#define _CRT_SECURE_NO_WARNINGS
#define VANGO_TEST_ROOT
#include <vangotest/asserts2.h>
#include <vangotest/bench.h>
#include "scripting/speech/graph.h"
#include "core/fs.h"


vango_test(bench_load_shmy1) {
    size_t sink = 0;

    vango_bench(10000, [&](){
        const auto g = shmy::speech::Graph::load_from_file("test/res/BrianDialogue.shmy");
        sink += (size_t)g.verts[0].n_lines;
    });

    const auto src = shmy::core::read_to_string("test/res/BrianDialogue.shmy").unwrap();

    vango_bench(10000, [&](){
        const auto g = shmy::speech::Graph::load_from_string(src);
        sink += (size_t)g.verts[0].n_lines;
    });

    printf("sink: %llu\n", sink);
}

vango_test(bench_load_shmy2) {
    size_t sink = 0;

    vango_bench(10000, [&](){
        const auto g = shmy::speech::Graph::load_from_file("test/res/BertDialogue.shmy");
        sink += (size_t)g.verts[0].n_lines;
    });

    const auto src = shmy::core::read_to_string("test/res/BertDialogue.shmy").unwrap();

    vango_bench(10000, [&](){
        const auto g = shmy::speech::Graph::load_from_string(src);
        sink += (size_t)g.verts[0].n_lines;
    });

    printf("sink: %llu\n", sink);
}


