#pragma once
#include <SFML/System.hpp>


class Time {
private:
    static sf::Clock clock;
    static sf::Time lasttime;

    inline static constexpr size_t FPS_FRAMES = 16;
    static float fps_arr[FPS_FRAMES];
    static size_t fps_index;

public:
    static float deltatime() { return lasttime.asSeconds(); }
    static int framerate() {
        fps_arr[fps_index++] = 1.f / deltatime();
        fps_index %= FPS_FRAMES;
        auto sum = 0.f;
        for (size_t i = 0; i < FPS_FRAMES; i++) {
            sum += fps_arr[i];
        }
        return (int)(sum / FPS_FRAMES);
    }
    static float framerate_precise() { return 1.f / deltatime(); }
    static sf::Time set_frame() { lasttime = clock.restart(); return lasttime; }
    static sf::Time reset() { return clock.reset(); }

    static void stop() { clock.stop(); }
    static void start() { clock.start(); }
};

