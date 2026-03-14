#pragma once
#include <miniaudio/miniaudio.h>
#include "audio/source.h"


namespace shmy { namespace audio {

struct Stream::ImplT {
    ma_decoder decoder;
};

} }
