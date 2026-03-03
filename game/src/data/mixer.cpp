#include "pch.h"
#include "mixer.h"
#include "bundler.h"


namespace shmy::data {

shmy::audio::Player Mixer::tracks[];
shmy::audio::Player Mixer::sounds[];
std::unordered_map<std::string, size_t> Mixer::track_list;


void Mixer::reset() {
    for (size_t i = 0; i < MAX_TRACKS; i++) {
        tracks[i].set_empty();
    }
    for (size_t i = 0; i < MAX_SOUNDS; i++) {
        sounds[i].set_empty();
    }
    track_list.clear();
}


shmy::audio::Player& Mixer::new_sound(const std::string& sound, bool stream) {
    shmy::audio::Player* p = nullptr;
    for (size_t i = 0; i < MAX_SOUNDS; i++) {
        if (sounds[i].is_empty() || !sounds[i].is_playing()) {
            if (stream) {
                sounds[i].set_source(Bundler::get_stream(sound));
            } else {
                sounds[i].set_source(Bundler::get_sound(sound));
            }
            p = &sounds[i];
        }
    }
    // if (!p) {
    //     throw std::runtime_error("audio error - maximum concurrent audio tracks exceeded (8)");
    // }

    return *p;
}

void Mixer::play_sound(const std::string& sound, const StartOptions& opts) {
    auto& p = new_sound(sound);
    p.start_after(opts.delay_millis);
    p.fade(0, opts.fadein_vol, opts.fadein_millis);
    p.set_looping(opts.loop);
}


shmy::audio::Player& Mixer::new_track(const std::string& track) {
    if (track_list.contains(track)) {
        return get_track(track);
    } else {
        for (size_t i = 0; i < MAX_TRACKS; i++) {
            if (tracks[i].is_empty() || !tracks[i].is_playing()) {
                track_list[track] = i;
                tracks[i].set_source(Bundler::get_stream(track));
                return tracks[i];
            }
        }
    }
    throw std::runtime_error("audio error - maximum concurrent audio tracks exceeded (8)");
}

void Mixer::play_track(const std::string& track, const StartOptions& opts) {
    auto& t = new_track(track);
    if (t.is_playing()) {
        t.fade(opts.fadein_vol, opts.fadein_millis);
        t.set_looping(opts.loop);
    } else {
        t.seek(0);
        t.stop_at(0); // needed for global miniaudio clock reasons
        t.start_after(opts.delay_millis);
        t.fade(0, opts.fadein_vol, opts.fadein_millis);
        t.set_looping(opts.loop);
        t.start();
    }
}

void Mixer::stop_track(const std::string& track, const StopOptions& opts) {
    if (!track_list.contains(track)) {
        return;
    }
    auto& t = get_track(track);
    t.fade(0.f, opts.fadeout_millis);
    t.stop_after(opts.delay_millis);
    t.stop_callback([](void* user_data, void*){
        Deleter* del = (Deleter*)user_data;
        del->list->erase(del->track);
        delete del;
    }, new Deleter{ &track_list, track });
}

shmy::audio::Player& Mixer::get_track(const std::string& track) {
    return tracks[track_list[track]];
}

}
