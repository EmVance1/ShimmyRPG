#define VANGO_TEST_ROOT
#include <vangotest/asserts2.h>
#include <audio/lib.h>
#include <stdio.h>
#include <Windows.h>


vango_test(shmy_buf_api) {
    vg_assert_eq(shmy::audio::create_context(), 0);

    {
        auto buf = shmy::audio::Buffer::load_file("test/res/test3.wav");
        vg_assert(buf.has_value());

        auto player = shmy::audio::Player(*buf);
        player.start();

        printf("press enter to continue...");
        getchar();
    }

    shmy::audio::destroy_context();
}

vango_test(shmy_stream_wav) {
    vg_assert_eq(shmy::audio::create_context(), 0);

    {
        auto buf = shmy::audio::Stream::open_file("test/res/Optimism.wav");
        vg_assert(buf.has_value());

        auto player = shmy::audio::Player(*buf);
        player.start();

        printf("press enter to continue...");
        getchar();

        player.stop();
    }

    shmy::audio::destroy_context();
}

vango_test(shmy_stream_ogg) {
    vg_assert_eq(shmy::audio::create_context(), 0);

    {
        auto buf = shmy::audio::Stream::open_file("test/res/Optimism.ogg");
        vg_assert(buf.has_value());

        auto player = shmy::audio::Player(*buf);
        player.start();

        printf("press enter to continue...");
        getchar();

        player.stop();
    }

    shmy::audio::destroy_context();
}

vango_test(shmy_stream_seek) {
    vg_assert_eq(shmy::audio::create_context(), 0);

    {
        auto buf = shmy::audio::Stream::open_file("test/res/Optimism.ogg");

        auto player = shmy::audio::Player(*buf);
        player.start();

        printf("press enter to continue...");
        getchar();

        player.seek(30.f);

        printf("press enter to stop...");
        getchar();

        player.stop();
    }

    shmy::audio::destroy_context();
}

vango_test(shmy_stream_spin) {
    vg_assert_eq(shmy::audio::create_context(), 0);

    {
        auto buf = shmy::audio::Stream::open_file("test/res/Optimism.ogg");

        auto player = shmy::audio::Player(*buf);
        player.set_spatialization_enabled(true);
        player.start();

        auto t = 0.f;
        while (t < 10.f) {
            auto x = std::cos(t);
            auto y = std::sin(t);
            player.set_position({ x, y, 0.f });
            Sleep(10);
            t += 0.05f;
        }

        printf("press enter to continue...");
        getchar();

        player.stop();
    }

    shmy::audio::destroy_context();
}

vango_test(shmy_stream_layer) {
    vg_assert_eq(shmy::audio::create_context(), 0);

    {
        auto buf1 = shmy::audio::Stream::open_file("test/res/Optimism.ogg");
        auto p1 = shmy::audio::Player(*buf1);
        p1.start();

        printf("press enter to continue...");
        getchar();

        auto buf2 = shmy::audio::Stream::open_file("test/res/Optimism.ogg");
        auto p2 = shmy::audio::Player(*buf2);
        p2.start();

        printf("press enter to stop...");
        getchar();

        p1.stop();
        p2.stop();
    }

    shmy::audio::destroy_context();
}

