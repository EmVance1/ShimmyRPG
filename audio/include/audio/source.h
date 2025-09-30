#pragma once
#include <filesystem>
#include <optional>
#include <cstdint>
#include "types.h"


namespace shmy { namespace audio {

class Source {
private:
    struct ImplT;
    ImplT* m_impl = nullptr;

private:
    Source(Source::ImplT* impl);

public:
    enum class Attenuation {
        None,
        Inverse,
        Linear,
        Exponential,
    };

public:
    Source(const Source& other);
    Source(Source&& other);
    ~Source();

    static std::optional<Source> load_from_file(const std::filesystem::path& path, bool spatial = false);
    static std::optional<Source> stream_from_file(const std::filesystem::path& path, bool spatial = false);

    std::optional<Source> try_copy() const;
    bool is_stream() const;

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

    Cone get_cone();
    Vec3 get_direction();
    Vec3 get_position();
    Vec3 get_velocity();
    Attenuation get_attenuation_model();
    float get_rolloff();
    float get_min_gain();
    float get_max_gain();
    float get_min_distance();
    float get_max_distance();
    float get_doppler_factor();

    void fade(float end_volume, uint32_t millis);

    void set_volume(float volume);
    float get_volume() const;

    void start();
    void stop();

    void start_after(uint32_t millis);
    void stop_after(uint32_t millis);
};

} }

