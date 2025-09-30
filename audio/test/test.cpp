#define VANGO_TEST_ROOT
#include <vangotest/asserts2.h>
#include <audio/lib.h>
#include <stdio.h>


vango_test(shmy_buf_api) {
    vg_assert_eq(shmy::audio::create_context(), 0);

    {
        auto buf1 = shmy::audio::Source::load_from_file("test/res/test1.wav");
        vg_assert(buf1.has_value());

        auto temp = buf1->try_copy();

        vg_assert(temp.has_value());

        auto buf2 = std::move(temp);

        buf2->start();

        printf("press enter to continue...");
        getchar();
    }

    shmy::audio::destroy_context();
}

vango_test(shmy_stream_api) {
    vg_assert_eq(shmy::audio::create_context(), 0);

    {
        auto buf = shmy::audio::Source::stream_from_file("test/res/Optimism.wav");
        vg_assert(buf.has_value());

        vg_assert(!buf->try_copy().has_value());

        buf->start();

        printf("press enter to continue...");
        getchar();

        buf->stop();

        printf("press enter to continue...");
        getchar();

        buf->start();

        printf("press enter to continue...");
        getchar();
    }

    shmy::audio::destroy_context();
}

vango_test(shmy_stream_ogg) {
    vg_assert_eq(shmy::audio::create_context(), 0);

    {
        auto buf = shmy::audio::Source::stream_from_file("test/res/Optimism.ogg");
        vg_assert(buf.has_value());

        vg_assert(!buf->try_copy().has_value());

        buf->start();

        printf("press enter to continue...");
        getchar();

        buf->stop();

        printf("press enter to continue...");
        getchar();

        buf->start();

        printf("press enter to continue...");
        getchar();
    }

    shmy::audio::destroy_context();
}

