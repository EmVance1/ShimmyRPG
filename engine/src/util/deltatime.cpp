#include "pch.h"
#include "util/deltatime.h"


sf::Clock Time::clock;
sf::Time Time::lasttime;
float Time::fps_arr[Time::FPS_FRAMES] = { 0 };
size_t Time::fps_index;

