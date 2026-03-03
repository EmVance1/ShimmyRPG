#include "pch.h"
#include "scripting/lua/conversions.h"
#include "data/mixer.h"


static int l_new_sound(lua_State* L);
static int l_play_sound(lua_State* L);

static int l_get_track(lua_State* L);
static int l_new_track(lua_State* L);
static int l_play_track(lua_State* L);
static int l_stop_track(lua_State* L);

void init_audio_api(lua_State* L) {
    lua_getglobal(L, "shmy");

    lua_newtable(L);

    lua_pushcfunction(L, &l_new_sound);
    lua_setfield(L, -2, "new_sound");
    lua_pushcfunction(L, &l_play_sound);
    lua_setfield(L, -2, "play_sound");

    lua_pushcfunction(L, &l_get_track);
    lua_setfield(L, -2, "get_track");
    lua_pushcfunction(L, &l_new_track);
    lua_setfield(L, -2, "new_track");
    lua_pushcfunction(L, &l_play_track);
    lua_setfield(L, -2, "play_track");
    lua_pushcfunction(L, &l_stop_track);
    lua_setfield(L, -2, "stop_track");

    lua_createtable(L, 0, 3);
    lua_pushinteger(L, (int)shmy::audio::Player::Attenuation::None);
    lua_setfield(L, -2, "NONE");
    lua_pushinteger(L, (int)shmy::audio::Player::Attenuation::Inverse);
    lua_setfield(L, -2, "INVERSE");
    lua_pushinteger(L, (int)shmy::audio::Player::Attenuation::Linear);
    lua_setfield(L, -2, "LINEAR");
    lua_pushinteger(L, (int)shmy::audio::Player::Attenuation::Exponential);
    lua_setfield(L, -2, "EXPONENTIAL");
    lua_setfield(L, -2, "attenuation");

    lua_setfield(L, -2, "audio");
}

static void create_player_table(lua_State* L, shmy::audio::Player* track);


static int l_new_sound(lua_State* L) {
    create_player_table(L, &shmy::data::Mixer::new_sound(lua_tostring(L, 1)));
    return 1;
}
static int l_play_sound(lua_State* L) {
    shmy::data::Mixer::play_sound(lua_tostring(L, 1));
    return 0;
}

static int l_get_track(lua_State* L) {
    create_player_table(L, &shmy::data::Mixer::get_track(lua_tostring(L, 1)));
    return 1;
}
static int l_new_track(lua_State* L) {
    create_player_table(L, &shmy::data::Mixer::new_track(lua_tostring(L, 1)));
    return 1;
}
static int l_play_track(lua_State* L) {
    shmy::data::Mixer::play_track(lua_tostring(L, 1));
    return 0;
}
static int l_stop_track(lua_State* L) {
    shmy::data::Mixer::stop_track(lua_tostring(L, 1));
    return 0;
}


static void lua_pushvec3(lua_State* L, const shmy::audio::Vec3& vec);
static shmy::audio::Vec3 lua_tovec3(lua_State* L, int idx);
static shmy::audio::Player* get_inner(lua_State* L, int idx) {
    lua_pushstring(L, "ptr");
    lua_gettable(L, idx);
    return static_cast<shmy::audio::Player*>(lua_touserdata(L, -1));
}


static int l_set_spatialization_enabled(lua_State* L) {
    auto player = get_inner(L, 1);
    player->set_spatialization_enabled(lua_toboolean(L, 2));
    return 0;
}
static int l_set_cone(lua_State* L) {
    auto player = get_inner(L, 1);
    auto cone = shmy::audio::Cone{};
    lua_getfield(L, 2, "radians_inner");
    cone.rads_inner = static_cast<float>(lua_tonumber(L, -1));
    lua_getfield(L, 2, "radians_outer");
    cone.rads_outer = static_cast<float>(lua_tonumber(L, -1));
    lua_getfield(L, 2, "gain_inner");
    cone.gain_outer = static_cast<float>(lua_tonumber(L, -1));
    lua_pop(L, 3);
    player->set_cone(cone);
    return 0;
}
static int l_set_direction(lua_State* L) {
    auto player = get_inner(L, 1);
    player->set_direction(lua_tovec3(L, 2));
    return 0;
}
static int l_set_position(lua_State* L) {
    auto player = get_inner(L, 1);
    player->set_position(lua_tovec3(L, 2));
    return 0;
}
static int l_set_velocity(lua_State* L) {
    auto player = get_inner(L, 1);
    player->set_velocity(lua_tovec3(L, 2));
    return 0;
}
static int l_set_attenuation_model(lua_State* L) {
    auto player = get_inner(L, 1);
    player->set_attenuation_model((shmy::audio::Player::Attenuation)lua_tointeger(L, 2));
    return 0;
}
static int l_set_rolloff(lua_State* L) {
    auto player = get_inner(L, 1);
    player->set_rolloff((float)lua_tonumber(L, 2));
    return 0;
}
static int l_set_min_gain(lua_State* L) {
    auto player = get_inner(L, 1);
    player->set_min_gain((float)lua_tonumber(L, 2));
    return 0;
}
static int l_set_max_gain(lua_State* L) {
    auto player = get_inner(L, 1);
    player->set_max_gain((float)lua_tonumber(L, 2));
    return 0;
}
static int l_set_min_distance(lua_State* L) {
    auto player = get_inner(L, 1);
    player->set_min_distance((float)lua_tonumber(L, 2));
    return 0;
}
static int l_set_max_distance(lua_State* L) {
    auto player = get_inner(L, 1);
    player->set_max_distance((float)lua_tonumber(L, 2));
    return 0;
}
static int l_set_doppler_factor(lua_State* L) {
    auto player = get_inner(L, 1);
    player->set_doppler_factor((float)lua_tonumber(L, 2));
    return 0;
}

static int l_get_spatialization_enabled(lua_State* L) {
    const auto enabled = get_inner(L, 1)->is_spatialization_enabled();
    lua_pushboolean(L, enabled);
    return 1;
}
static int l_get_cone(lua_State* L) {
    const auto cone = get_inner(L, 1)->get_cone();
    lua_newtable(L);
    lua_pushnumber(L, (lua_Number)cone.rads_inner);
    lua_setfield(L, -2, "radians_inner");
    lua_pushnumber(L, (lua_Number)cone.rads_outer);
    lua_setfield(L, -2, "radians_outer");
    lua_pushnumber(L, (lua_Number)cone.gain_outer);
    lua_setfield(L, -2, "gain_outer");
    return 1;
}
static int l_get_direction(lua_State* L) {
    const auto dir = get_inner(L, 1)->get_direction();
    lua_pushvec3(L, dir);
    return 1;
}
static int l_get_position(lua_State* L) {
    const auto pos = get_inner(L, 1)->get_position();
    lua_pushvec3(L, pos);
    return 1;
}
static int l_get_velocity(lua_State* L) {
    const auto vel = get_inner(L, 1)->get_velocity();
    lua_pushvec3(L, vel);
    return 1;
}
static int l_get_attenuation_model(lua_State* L) {
    const auto att = get_inner(L, 1)->get_attenuation_model();
    lua_pushinteger(L, (int)att);
    return 1;
}
static int l_get_rolloff(lua_State* L) {
    const auto roll = get_inner(L, 1)->get_rolloff();
    lua_pushnumber(L, (lua_Number)roll);
    return 1;
}
static int l_get_min_gain(lua_State* L) {
    const auto gain = get_inner(L, 1)->get_min_gain();
    lua_pushnumber(L, (lua_Number)gain);
    return 1;
}
static int l_get_max_gain(lua_State* L) {
    const auto gain = get_inner(L, 1)->get_max_gain();
    lua_pushnumber(L, (lua_Number)gain);
    return 1;
}
static int l_get_min_distance(lua_State* L) {
    const auto dist = get_inner(L, 1)->get_min_distance();
    lua_pushnumber(L, (lua_Number)dist);
    return 1;
}
static int l_get_max_distance(lua_State* L) {
    const auto dist = get_inner(L, 1)->get_max_distance();
    lua_pushnumber(L, (lua_Number)dist);
    return 1;
}
static int l_get_doppler_factor(lua_State* L) {
    const auto fac = get_inner(L, 1)->get_doppler_factor();
    lua_pushnumber(L, (lua_Number)fac);
    return 1;
}

static void create_player_table(lua_State* L, shmy::audio::Player* audio) {
    lua_newtable(L);

    lua_pushlightuserdata(L, audio);
    lua_setfield(L, -2, "ptr");

    lua_pushcfunction(L, &l_set_spatialization_enabled);
    lua_setfield(L, -2, "set_spatialization_enabled");
    lua_pushcfunction(L, &l_set_cone);
    lua_setfield(L, -2, "set_cone");
    lua_pushcfunction(L, &l_set_direction);
    lua_setfield(L, -2, "set_direction");
    lua_pushcfunction(L, &l_set_position);
    lua_setfield(L, -2, "set_position");
    lua_pushcfunction(L, &l_set_velocity);
    lua_setfield(L, -2, "set_velocity");
    lua_pushcfunction(L, &l_set_attenuation_model);
    lua_setfield(L, -2, "set_attenuation_model");
    lua_pushcfunction(L, &l_set_rolloff);
    lua_setfield(L, -2, "set_rolloff");
    lua_pushcfunction(L, &l_set_min_gain);
    lua_setfield(L, -2, "set_min_gain");
    lua_pushcfunction(L, &l_set_max_gain);
    lua_setfield(L, -2, "set_max_gain");
    lua_pushcfunction(L, &l_set_min_distance);
    lua_setfield(L, -2, "set_min_distance");
    lua_pushcfunction(L, &l_set_max_distance);
    lua_setfield(L, -2, "set_max_distance");
    lua_pushcfunction(L, &l_set_doppler_factor);
    lua_setfield(L, -2, "set_doppler_factor");

    lua_pushcfunction(L, &l_get_spatialization_enabled);
    lua_setfield(L, -2, "get_spatialization_enabled");
    lua_pushcfunction(L, &l_get_cone);
    lua_setfield(L, -2, "get_cone");
    lua_pushcfunction(L, &l_get_direction);
    lua_setfield(L, -2, "get_direction");
    lua_pushcfunction(L, &l_get_position);
    lua_setfield(L, -2, "get_position");
    lua_pushcfunction(L, &l_get_velocity);
    lua_setfield(L, -2, "get_velocity");
    lua_pushcfunction(L, &l_get_attenuation_model);
    lua_setfield(L, -2, "get_attenuation_model");
    lua_pushcfunction(L, &l_get_rolloff);
    lua_setfield(L, -2, "get_rolloff");
    lua_pushcfunction(L, &l_get_min_gain);
    lua_setfield(L, -2, "get_min_gain");
    lua_pushcfunction(L, &l_get_max_gain);
    lua_setfield(L, -2, "get_max_gain");
    lua_pushcfunction(L, &l_get_min_distance);
    lua_setfield(L, -2, "get_min_distance");
    lua_pushcfunction(L, &l_get_max_distance);
    lua_setfield(L, -2, "get_max_distance");
    lua_pushcfunction(L, &l_get_doppler_factor);
    lua_setfield(L, -2, "get_doppler_factor");

    // void fade(float start_volume, float end_volume, uint32_t millis);
    // void fade(float end_volume, uint32_t millis);

    // void set_volume(float volume);
    // float get_volume() const;

    // void start();
    // void stop();
    // void set_looping(bool loop);

    // bool is_playing() const;
    // bool is_looping() const;

    // void start_at(uint32_t millis);
    // void start_after(uint32_t millis);
    // void stop_at(uint32_t millis);
    // void stop_after(uint32_t millis);
    // void stop_callback(StopCallback callback, void* user_data);

    // void seek(float seconds);
}


static void lua_pushvec3(lua_State* L, const shmy::audio::Vec3& vec) {
    lua_newtable(L);
    lua_pushnumber(L, (lua_Number)vec.x);
    lua_setfield(L, -2, "x");
    lua_pushnumber(L, (lua_Number)vec.y);
    lua_setfield(L, -2, "y");
    lua_pushnumber(L, (lua_Number)vec.z);
    lua_setfield(L, -2, "z");
}

static shmy::audio::Vec3 lua_tovec3(lua_State* L, int idx) {
    lua_getfield(L, idx, "x");
    const float x = static_cast<float>(lua_tonumber(L, -1));
    lua_getfield(L, idx, "y");
    const float y = static_cast<float>(lua_tonumber(L, -1));
    lua_getfield(L, idx, "z");
    const float z = static_cast<float>(lua_tonumber(L, -1));
    lua_pop(L, 3);
    return shmy::audio::Vec3(x, y, z);
}

