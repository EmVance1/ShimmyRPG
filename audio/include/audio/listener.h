#pragma once
#include <cstdint>
#include "types.h"


namespace shmy { namespace audio {

class Listener {
    inline static uint32_t main_listener = 0;
    inline static bool single_listener = true;

public:
    void set_master_volume(float volume);

    void set_listener_count(uint32_t count);

    // ensures only a single 'main' listener is ever enabled
    void set_listen_from_single(uint32_t index);
    // allow closest listener model
    void set_listen_from_enabled();
    void set_listen_from_all();

    void set_main_cone(Cone cone);
    void set_main_direction(Vec3 dir);
    void set_main_position (Vec3 pos);
    void set_main_velocity (Vec3 vel);
    void set_main_world_up (Vec3 up);

    Cone get_main_cone();
    Vec3 get_main_direction();
    Vec3 get_main_position();
    Vec3 get_main_velocity();
    Vec3 get_main_world_up();

    void set_enabled(uint32_t listener, bool enabled);
    void set_cone(uint32_t listener, Cone cone);
    void set_direction(uint32_t listener, Vec3 dir);
    void set_position(uint32_t listener, Vec3 pos);
    void set_velocity(uint32_t listener, Vec3 vel);
    void set_world_up(uint32_t listener, Vec3 up);

    bool is_enabled(uint32_t listener);
    Cone get_cone(uint32_t listener);
    Vec3 get_direction(uint32_t listener);
    Vec3 get_position(uint32_t listener);
    Vec3 get_velocity(uint32_t listener);
    Vec3 get_world_up(uint32_t listener);
};

} }

