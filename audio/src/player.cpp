#include "audio/player.h"
#include <miniaudio/miniaudio.h>
#include <iostream>
#include "impl.h"


namespace shmy { namespace audio {

ma_engine* get_audio_engine_ptr();

#ifdef VANGO_DEBUG
#define MA_CHECK(x) do { result = x; if (result != MA_SUCCESS) { \
    std::cerr << "miniaudio error: " << ma_result_description(result) << "\n"; exit(1); } } while (0)
#else
#define MA_CHECK(x) x
#endif


struct Player::ImplT {
    enum class SourceType { Empty, Buffer, Stream } source_type = SourceType::Empty;
    union {
        ma_audio_buffer_ref buffer_ref;
        ma_decoder* stream_ref;
    } source{};
    ma_sound player;
};


void Player::cleanup_source() {
    if (m_impl->source_type == ImplT::SourceType::Empty) return;
    ma_sound_uninit(&m_impl->player);
    if (m_impl->source_type == ImplT::SourceType::Buffer) {
        ma_audio_buffer_ref_uninit(&m_impl->source.buffer_ref);
    }
}

void* Player::get_sound() {
    return &m_impl->player;
}

const void* Player::get_sound() const {
    return &m_impl->player;
}

Player::Player() : m_impl(new ImplT()) {}

Player::Player(const Buffer& buffer) : m_impl(new ImplT()) {
    set_source(buffer);
}
Player::Player(Stream& stream) : m_impl(new ImplT()) {
    set_source(stream);
}
Player::Player(const Player& other) : m_impl(new ImplT()) {
    if (other.m_impl->source_type == ImplT::SourceType::Empty) return;
    if (other.m_impl->source_type == ImplT::SourceType::Stream) {
        throw std::runtime_error("attempt to copy non-copyable datastream");
    }
    m_impl->source_type = ImplT::SourceType::Buffer;
    ma_result result;
    MA_CHECK(ma_audio_buffer_ref_init(
        other.m_impl->source.buffer_ref.format,
        other.m_impl->source.buffer_ref.channels,
        other.m_impl->source.buffer_ref.pData,
        other.m_impl->source.buffer_ref.sizeInFrames,
        &m_impl->source.buffer_ref
    ));
    MA_CHECK(ma_sound_init_from_data_source(get_audio_engine_ptr(), &m_impl->source.buffer_ref, 0, NULL, &m_impl->player));
}
Player::Player(Player&& other) noexcept
    : m_impl(other.m_impl)
{
    other.m_impl = nullptr;
}
Player::~Player() {
    cleanup_source();
    delete m_impl;
}

Player& Player::operator=(const Player& other) {
    cleanup_source();
    m_impl->source_type = other.m_impl->source_type;

    if (other.m_impl->source_type == ImplT::SourceType::Empty) return *this;
    if (other.m_impl->source_type == ImplT::SourceType::Stream) {
        throw std::runtime_error("attempt to copy non-copyable datastream");
    }

    ma_result result;
    MA_CHECK(ma_audio_buffer_ref_init(
        other.m_impl->source.buffer_ref.format,
        other.m_impl->source.buffer_ref.channels,
        other.m_impl->source.buffer_ref.pData,
        other.m_impl->source.buffer_ref.sizeInFrames,
        &m_impl->source.buffer_ref
    ));
    MA_CHECK(ma_sound_init_from_data_source(get_audio_engine_ptr(), &m_impl->source.buffer_ref, 0, NULL, &m_impl->player));
    return *this;
}

Player& Player::operator=(Player&& other) noexcept {
    this->~Player();
    m_impl = other.m_impl;
    other.m_impl = nullptr;
    return *this;
}


bool Player::is_empty() const {
    return m_impl->source_type == ImplT::SourceType::Empty;
}
void Player::set_empty() {
    cleanup_source();
    m_impl->source_type = ImplT::SourceType::Empty;
}
void Player::set_source(const Buffer& buffer) {
    cleanup_source();
    m_impl->source_type = ImplT::SourceType::Buffer;

    ma_result result;
    MA_CHECK(ma_audio_buffer_ref_init(
        (ma_format)buffer.format,
        buffer.channels,
        buffer.pcm.data(),
        buffer.frame_count,
        &m_impl->source.buffer_ref
    ));
    MA_CHECK(ma_sound_init_from_data_source(get_audio_engine_ptr(), &m_impl->source.buffer_ref, 0, NULL, &m_impl->player));
}
void Player::set_source(Stream& stream) {
    cleanup_source();
    m_impl->source_type = ImplT::SourceType::Stream;

    ma_result result;
    m_impl->source.stream_ref = &stream.m_impl->decoder;
    MA_CHECK(ma_sound_init_from_data_source(get_audio_engine_ptr(), m_impl->source.stream_ref, 0, NULL, &m_impl->player));
}


void Player::set_spatialization_enabled(bool enabled) {
    ma_sound_set_spatialization_enabled((ma_sound*)get_sound(), enabled);
}
void Player::set_cone(Cone cone) {
    ma_sound_set_cone((ma_sound*)get_sound(), cone.rads_inner, cone.rads_outer, cone.gain_outer);
}
void Player::set_direction(Vec3 dir) {
    ma_sound_set_direction((ma_sound*)get_sound(), dir.x, dir.y, dir.z);
}
void Player::set_position(Vec3 pos) {
    ma_sound_set_position((ma_sound*)get_sound(), pos.x, pos.y, pos.z);
}
void Player::set_velocity(Vec3 vel) {
    ma_sound_set_velocity((ma_sound*)get_sound(), vel.x, vel.y, vel.z);
}
void Player::set_attenuation_model(Attenuation model) {
    switch (model) {
    case Attenuation::None:
        ma_sound_set_attenuation_model((ma_sound*)get_sound(), ma_attenuation_model_none);
        break;
    case Attenuation::Inverse:
        ma_sound_set_attenuation_model((ma_sound*)get_sound(), ma_attenuation_model_inverse);
        break;
    case Attenuation::Linear:
        ma_sound_set_attenuation_model((ma_sound*)get_sound(), ma_attenuation_model_linear);
        break;
    case Attenuation::Exponential:
        ma_sound_set_attenuation_model((ma_sound*)get_sound(), ma_attenuation_model_exponential);
        break;
    }
}
void Player::set_rolloff(float rolloff) {
    ma_sound_set_rolloff((ma_sound*)get_sound(), rolloff);
}
void Player::set_min_gain(float min) {
    ma_sound_set_min_gain((ma_sound*)get_sound(), min);
}
void Player::set_max_gain(float max) {
    ma_sound_set_max_gain((ma_sound*)get_sound(), max);
}
void Player::set_min_distance(float min) {
    ma_sound_set_min_distance((ma_sound*)get_sound(), min);
}
void Player::set_max_distance(float max) {
    ma_sound_set_max_distance((ma_sound*)get_sound(), max);
}
void Player::set_doppler_factor(float factor) {
    ma_sound_set_doppler_factor((ma_sound*)get_sound(), factor);
}

bool Player::is_spatialization_enabled() const {
    return ma_sound_is_spatialization_enabled((ma_sound*)get_sound());
}
Cone Player::get_cone() const {
    Cone cone;
    ma_sound_get_cone((ma_sound*)get_sound(), &cone.rads_inner, &cone.rads_outer, &cone.gain_outer);
    return cone;
}
Vec3 Player::get_direction() const {
    const auto vec = ma_sound_get_direction((ma_sound*)get_sound());
    return { vec.x, vec.y, vec.z };
}
Vec3 Player::get_position() const {
    const auto vec = ma_sound_get_position((ma_sound*)get_sound());
    return { vec.x, vec.y, vec.z };
}
Vec3 Player::get_velocity() const {
    const auto vec = ma_sound_get_velocity((ma_sound*)get_sound());
    return { vec.x, vec.y, vec.z };
}
Player::Attenuation Player::get_attenuation_model() const {
    const auto model = ma_sound_get_attenuation_model((ma_sound*)get_sound());
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
float Player::get_rolloff() const {
    return ma_sound_get_rolloff((ma_sound*)get_sound());
}
float Player::get_min_gain() const {
    return ma_sound_get_min_gain((ma_sound*)get_sound());
}
float Player::get_max_gain() const {
    return ma_sound_get_max_gain((ma_sound*)get_sound());
}
float Player::get_min_distance() const {
    return ma_sound_get_min_distance((ma_sound*)get_sound());
}
float Player::get_max_distance() const {
    return ma_sound_get_max_distance((ma_sound*)get_sound());
}
float Player::get_doppler_factor() const {
    return ma_sound_get_doppler_factor((ma_sound*)get_sound());
}


void Player::fade(float start_volume, float end_volume, uint32_t millis) {
    ma_sound_set_fade_in_milliseconds((ma_sound*)get_sound(), start_volume, end_volume, millis);
}

void Player::fade(float end_volume, uint32_t millis) {
    ma_sound_set_fade_in_milliseconds((ma_sound*)get_sound(), -1, end_volume, millis);
}


void Player::set_volume(float volume) {
    ma_sound_set_volume((ma_sound*)get_sound(), volume);
}

float Player::get_volume() const {
    return ma_sound_get_volume((ma_sound*)get_sound());
}


void Player::start() {
    ma_sound_start((ma_sound*)get_sound());
}

void Player::stop() {
    ma_sound_stop((ma_sound*)get_sound());
}

void Player::set_looping(bool loop) {
    ma_sound_set_looping((ma_sound*)get_sound(), loop);
}

bool Player::is_playing() const {
    return ma_sound_is_playing((ma_sound*)get_sound());
}

bool Player::is_looping() const {
    return ma_sound_is_looping((ma_sound*)get_sound());
}


void Player::start_at(uint32_t millis) {
    ma_sound_set_start_time_in_milliseconds((ma_sound*)get_sound(), millis);
}
void Player::start_after(uint32_t millis) {
    ma_sound_set_start_time_in_milliseconds((ma_sound*)get_sound(), ma_engine_get_time_in_milliseconds(get_audio_engine_ptr()) + millis);
}
void Player::stop_at(uint32_t millis) {
    if (millis == 0) {
        ma_sound_set_stop_time_in_milliseconds((ma_sound*)get_sound(), UINT64_MAX);
    } else {
        ma_sound_set_stop_time_in_milliseconds((ma_sound*)get_sound(), millis);
    }
}
void Player::stop_after(uint32_t millis) {
    ma_sound_set_stop_time_in_milliseconds((ma_sound*)get_sound(), ma_engine_get_time_in_milliseconds(get_audio_engine_ptr()) + millis);
}
void Player::stop_callback(StopCallback callback, void* user_data) {
    ma_sound_set_end_callback((ma_sound*)get_sound(), (ma_sound_end_proc)callback, user_data);
}


void Player::seek(float seconds) {
    ma_sound_seek_to_second((ma_sound*)get_sound(), seconds);
}

} }
