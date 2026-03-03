#pragma once
#include <audio/lib.h>
#include <unordered_map>
#include <string>


namespace shmy::data {

class Mixer {
public:
    constexpr inline static size_t MAX_TRACKS = 8;
    constexpr inline static size_t MAX_SOUNDS = 512;

    struct StartOptions {
        uint32_t delay_millis = 0;
        uint32_t fadein_millis = 0;
        float fadein_vol = 1.f;
        bool loop = true;
        bool stream = false;
        bool spatial = false;
        shmy::audio::Vec3 position;

        static StartOptions Default() { return StartOptions(); }
    };
    struct StopOptions {
        uint32_t delay_millis = 0;
        uint32_t fadeout_millis = 0;

        static StopOptions Default() { return StopOptions(); }
    };

private:
    static shmy::audio::Player tracks[MAX_TRACKS];
    static shmy::audio::Player sounds[MAX_SOUNDS];
    static std::unordered_map<std::string, size_t> track_list;
    struct Deleter {
        std::unordered_map<std::string, size_t>* list;
        std::string track;
    };

public:
    static void reset();

    static shmy::audio::Player& new_sound(const std::string& sound, bool stream = false);
    static void play_sound(const std::string& sound, const StartOptions& opts = StartOptions::Default());

    static shmy::audio::Player& new_track(const std::string& track);
    static void play_track(const std::string& track, const StartOptions& opts = StartOptions::Default());
    static void stop_track(const std::string& track, const StopOptions& opts = StopOptions::Default());
    static shmy::audio::Player& get_track(const std::string& track);
};

}
