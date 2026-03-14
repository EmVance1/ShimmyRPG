#pragma once
#include <cstdint>
#include "source.h"
#include "types.h"


namespace shmy { namespace audio {

class Allocator {
public:
    virtual void* alloc(size_t) = 0;
    virtual void dealloc(void*) = 0;
};

class Player {
private:
    struct ImplT;
    ImplT* m_impl = nullptr;

    void cleanup_source();

    void* get_sound();
    const void* get_sound() const;

public:
    enum class Attenuation {
        None,
        Inverse,
        Linear,
        Exponential,
    };
    using StopCallback = void(*)(void*, void*);

public:
    Player();
    explicit Player(const Buffer& buffer);
    explicit Player(Stream& stream);
    Player(const Player& other);
    Player(Player&& other) noexcept;
    ~Player();

    Player& operator=(const Player& other);
    Player& operator=(Player&& other) noexcept;

    bool is_empty() const;
    void set_empty();
    void set_source(const Buffer& buffer);
    void set_source(Stream& stream);

    void set_spatialization_enabled(bool enabled);
    void set_cone(Cone cone);
    void set_direction(Vec3 dir);
    void set_position(Vec3 pos);
    void set_velocity(Vec3 vel);
    void set_attenuation_model(Attenuation model);
    void set_rolloff(float rolloff);
    void set_min_gain(float min);
    void set_max_gain(float max);
    void set_min_distance(float min);
    void set_max_distance(float max);
    void set_doppler_factor(float factor);

    bool is_spatialization_enabled() const;
    Cone get_cone() const;
    Vec3 get_direction() const;
    Vec3 get_position() const;
    Vec3 get_velocity() const;
    Attenuation get_attenuation_model() const;
    float get_rolloff() const;
    float get_min_gain() const;
    float get_max_gain() const;
    float get_min_distance() const;
    float get_max_distance() const;
    float get_doppler_factor() const;

    void fade(float start_volume, float end_volume, uint32_t millis);
    void fade(float end_volume, uint32_t millis);

    void set_volume(float volume);
    float get_volume() const;

    void start();
    void stop();
    void set_looping(bool loop);

    bool is_playing() const;
    bool is_looping() const;

    void start_at(uint32_t millis);
    void start_after(uint32_t millis);
    void stop_at(uint32_t millis);
    void stop_after(uint32_t millis);
    void stop_callback(StopCallback callback, void* user_data);

    void seek(float seconds);
};

} }

