#include "audio/listener.h"
#include <miniaudio/miniaudio.h>


namespace shmy { namespace audio {

ma_engine* get_audio_engine_ptr();


void Listener::set_master_volume(float volume) {
    ma_engine_set_volume(get_audio_engine_ptr(), volume);
};

void Listener::set_listener_count(uint32_t count) {
    get_audio_engine_ptr()->listenerCount = count;
}

void Listener::set_listen_from_single(uint32_t index) {
    single_listener = true;
    for (int i = 0; i < get_audio_engine_ptr()->listenerCount; i++) {
        ma_engine_listener_set_enabled(get_audio_engine_ptr(), i, i == index);
    }
    main_listener = index;
}

void Listener::set_listen_from_enabled() {
    single_listener = false;
}

void Listener::set_listen_from_all() {
    single_listener = false;
    for (int i = 0; i < get_audio_engine_ptr()->listenerCount; i++) {
        ma_engine_listener_set_enabled(get_audio_engine_ptr(), i, true);
    }
}


void Listener::set_main_cone(Cone cone) {
    Listener::set_cone(main_listener, cone);
}

void Listener::set_main_direction(Vec3 dir) {
    Listener::set_direction(main_listener, dir);
}

void Listener::set_main_position(Vec3 pos) {
    Listener::set_position(main_listener, pos);
}

void Listener::set_main_velocity(Vec3 vel) {
    Listener::set_velocity(main_listener, vel);
}

void Listener::set_main_world_up(Vec3 up) {
    Listener::set_world_up(main_listener, up);
}


Cone Listener::get_main_cone() {
    return Listener::get_cone(main_listener);
}

Vec3 Listener::get_main_direction() {
    return Listener::get_direction(main_listener);
}

Vec3 Listener::get_main_position() {
    return Listener::get_position(main_listener);
}

Vec3 Listener::get_main_velocity() {
    return Listener::get_velocity(main_listener);
}

Vec3 Listener::get_main_world_up() {
    return Listener::get_world_up(main_listener);
}


void Listener::set_enabled(uint32_t listener, bool enabled) {
    if (single_listener) {
        ma_engine_listener_set_enabled(get_audio_engine_ptr(), main_listener, false);
    }
    main_listener = listener;
    ma_engine_listener_set_enabled(get_audio_engine_ptr(), listener, enabled);
}

void Listener::set_cone(uint32_t listener, Cone cone) {
    ma_engine_listener_set_cone(get_audio_engine_ptr(), listener, cone.rads_inner, cone.rads_outer, cone.gain_outer);
}

void Listener::set_direction(uint32_t listener, Vec3 dir) {
    ma_engine_listener_set_direction(get_audio_engine_ptr(), listener, dir.x, dir.y, dir.z);
}

void Listener::set_position(uint32_t listener, Vec3 pos) {
    ma_engine_listener_set_position(get_audio_engine_ptr(), listener, pos.x, pos.y, pos.z);
}

void Listener::set_velocity(uint32_t listener, Vec3 vel) {
    ma_engine_listener_set_velocity(get_audio_engine_ptr(), listener, vel.x, vel.y, vel.z);
}

void Listener::set_world_up(uint32_t listener, Vec3 up) {
    ma_engine_listener_set_world_up(get_audio_engine_ptr(), listener, up.x, up.y, up.z);
}


bool Listener::is_enabled(uint32_t listener) {
    return ma_engine_listener_is_enabled(get_audio_engine_ptr(), listener);
}

Cone Listener::get_cone(uint32_t listener) {
    Cone cone;
    ma_engine_listener_get_cone(get_audio_engine_ptr(), listener, &cone.rads_inner, &cone.rads_outer, &cone.gain_outer);
    return cone;
}

Vec3 Listener::get_direction(uint32_t listener) {
    const auto vec = ma_engine_listener_get_direction(get_audio_engine_ptr(), listener);
    return { vec.x, vec.y, vec.z };
}

Vec3 Listener::get_position(uint32_t listener) {
    const auto vec = ma_engine_listener_get_position(get_audio_engine_ptr(), listener);
    return { vec.x, vec.y, vec.z };
}

Vec3 Listener::get_velocity(uint32_t listener) {
    const auto vec = ma_engine_listener_get_velocity(get_audio_engine_ptr(), listener);
    return { vec.x, vec.y, vec.z };
}

Vec3 Listener::get_world_up(uint32_t listener) {
    const auto vec = ma_engine_listener_get_world_up(get_audio_engine_ptr(), listener);
    return { vec.x, vec.y, vec.z };
}


} }

