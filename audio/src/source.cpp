#include "audio/source.h"
#include <miniaudio/miniaudio.h>
#include <stdexcept>
#ifdef VANGO_DEBUG
#include <iostream>
#endif


namespace shmy { namespace audio {

ma_engine* get_audio_engine_ptr();


struct Source::ImplT {
    ma_sound sound;
    bool streamed;
};


Source::Source(Source::ImplT* impl)
    : m_impl(std::move(impl))
{}

Source::Source(const Source& other)
    : m_impl(new Source::ImplT())
{
    if (other.m_impl->streamed) {
        throw std::runtime_error("cannot copy construct a streaming audio source");
    }
    ma_sound_init_copy(get_audio_engine_ptr(), &other.m_impl->sound, 0, NULL, &m_impl->sound);
    m_impl->streamed = false;
}

Source::Source(Source&& other)
    : m_impl(other.m_impl)
{
    other.m_impl = nullptr;
}

Source::~Source() {
    if (m_impl) {
        ma_sound_uninit(&m_impl->sound);
        delete m_impl;
    }
}


std::optional<Source> Source::load_from_file(const std::filesystem::path& path, bool spatial) {
    auto src = Source{ new Source::ImplT() };
    src.m_impl->streamed = false;
    uint32_t flags = MA_SOUND_FLAG_DECODE;
    if (!spatial) { flags |= MA_SOUND_FLAG_NO_SPATIALIZATION; }

#ifdef _WIN32
    ma_result result = ma_sound_init_from_file_w(get_audio_engine_ptr(), path.c_str(), flags, NULL, NULL, &src.m_impl->sound);
#else
    ma_result result = ma_sound_init_from_file(get_audio_engine_ptr(), path.c_str(), flags, NULL, NULL, &src.m_impl->sound);
#endif
    if (result == MA_SUCCESS) {
        return src;
    } else {
#ifdef VANGO_DEBUG
        std::cerr << "miniaudio internal error: " << ma_result_description(result) << "\n";
#endif
        return {};
    }
}

std::optional<Source> Source::stream_from_file(const std::filesystem::path& path, bool spatial) {
    auto src = Source{ new Source::ImplT() };
    src.m_impl->streamed = true;
    uint32_t flags = MA_SOUND_FLAG_STREAM;
    if (!spatial) { flags |= MA_SOUND_FLAG_NO_SPATIALIZATION; }

#ifdef _WIN32
    ma_result result = ma_sound_init_from_file_w(get_audio_engine_ptr(), path.c_str(), flags, NULL, NULL, &src.m_impl->sound);
#else
    ma_result result = ma_sound_init_from_file(get_audio_engine_ptr(), path.c_str(), flags, NULL, NULL, &src.m_impl->sound);
#endif
    if (result == MA_SUCCESS) {
        return src;
    } else {
#ifdef VANGO_DEBUG
        std::cerr << "miniaudio internal error: " << ma_result_description(result) << "\n";
#endif
        return {};
    }
}

std::optional<Source> Source::try_copy() const {
    if (m_impl->streamed) return {};
    auto other = Source{ new Source::ImplT() };
    ma_sound_init_copy(get_audio_engine_ptr(), &m_impl->sound, 0, NULL, &other.m_impl->sound);
    other.m_impl->streamed = false;
    return other;
}

bool Source::is_stream() const {
    return m_impl->streamed;
}


void Source::set_cone(Cone cone) {
    ma_sound_set_cone(&m_impl->sound, cone.rads_inner, cone.rads_outer, cone.gain_outer);
}
void Source::set_direction(Vec3 dir) {
    ma_sound_set_direction(&m_impl->sound, dir.x, dir.y, dir.z);
}
void Source::set_position(Vec3 pos) {
    ma_sound_set_position(&m_impl->sound, pos.x, pos.y, pos.z);
}
void Source::set_velocity(Vec3 vel) {
    ma_sound_set_velocity(&m_impl->sound, vel.x, vel.y, vel.z);
}
void Source::set_attenuation_model(Attenuation model) {
    switch (model) {
    case Attenuation::None:
        ma_sound_set_attenuation_model(&m_impl->sound, ma_attenuation_model_none);
        break;
    case Attenuation::Inverse:
        ma_sound_set_attenuation_model(&m_impl->sound, ma_attenuation_model_inverse);
        break;
    case Attenuation::Linear:
        ma_sound_set_attenuation_model(&m_impl->sound, ma_attenuation_model_linear);
        break;
    case Attenuation::Exponential:
        ma_sound_set_attenuation_model(&m_impl->sound, ma_attenuation_model_exponential);
        break;
    }
}
void Source::set_rolloff(float rolloff) {
    ma_sound_set_rolloff(&m_impl->sound, rolloff);
}
void Source::set_min_gain(float min) {
    ma_sound_set_min_gain(&m_impl->sound, min);
}
void Source::set_max_gain(float max) {
    ma_sound_set_max_gain(&m_impl->sound, max);
}
void Source::set_min_distance(float min) {
    ma_sound_set_min_distance(&m_impl->sound, min);
}
void Source::set_max_distance(float max) {
    ma_sound_set_max_distance(&m_impl->sound, max);
}
void Source::set_doppler_factor(float factor) {
    ma_sound_set_doppler_factor(&m_impl->sound, factor);
}

Cone Source::get_cone() {
    Cone cone;
    ma_sound_get_cone(&m_impl->sound, &cone.rads_inner, &cone.rads_outer, &cone.gain_outer);
    return cone;
}
Vec3 Source::get_direction() {
    const auto vec = ma_sound_get_direction(&m_impl->sound);
    return { vec.x, vec.y, vec.z };
}
Vec3 Source::get_position() {
    const auto vec = ma_sound_get_position(&m_impl->sound);
    return { vec.x, vec.y, vec.z };
}
Vec3 Source::get_velocity() {
    const auto vec = ma_sound_get_velocity(&m_impl->sound);
    return { vec.x, vec.y, vec.z };
}
Source::Attenuation Source::get_attenuation_model() {
    const auto model = ma_sound_get_attenuation_model(&m_impl->sound);
    switch (model) {
    case ma_attenuation_model_none:
        return Attenuation::None;
    case ma_attenuation_model_inverse:
        return Attenuation::Inverse;
    case ma_attenuation_model_linear:
        return Attenuation::Linear;
    case ma_attenuation_model_exponential:
        return Attenuation::Exponential;
    }
    return Attenuation::None;
}
float Source::get_rolloff() {
    return ma_sound_get_rolloff(&m_impl->sound);
}
float Source::get_min_gain() {
    return ma_sound_get_min_gain(&m_impl->sound);
}
float Source::get_max_gain() {
    return ma_sound_get_max_gain(&m_impl->sound);
}
float Source::get_min_distance() {
    return ma_sound_get_min_distance(&m_impl->sound);
}
float Source::get_max_distance() {
    return ma_sound_get_max_distance(&m_impl->sound);
}
float Source::get_doppler_factor() {
    return ma_sound_get_doppler_factor(&m_impl->sound);
}


void Source::fade(float end_volume, uint32_t millis) {
    ma_sound_set_fade_in_milliseconds(&m_impl->sound, -1, end_volume, millis);
}


void Source::set_volume(float volume) {
    ma_sound_set_volume(&m_impl->sound, volume);
}

float Source::get_volume() const {
    return ma_sound_get_volume(&m_impl->sound);
}


void Source::start() {
    ma_sound_start(&m_impl->sound);
}

void Source::stop() {
    ma_sound_stop(&m_impl->sound);
}

void Source::start_after(uint32_t millis) {
    ma_sound_set_start_time_in_milliseconds(&m_impl->sound, ma_engine_get_time_in_milliseconds(get_audio_engine_ptr()) + millis);
}
void Source::stop_after(uint32_t millis) {
    ma_sound_set_stop_time_in_milliseconds(&m_impl->sound, ma_engine_get_time_in_milliseconds(get_audio_engine_ptr()) + millis);
}


} }

