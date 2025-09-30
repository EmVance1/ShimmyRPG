#define MINIAUDIO_IMPLEMENTATION
#include <miniaudio/miniaudio.h>
#include <miniaudio/extras/decoders/libvorbis/miniaudio_libvorbis.c>


namespace shmy { namespace audio {

static ma_resource_manager s_shmy_audio_resources;
static ma_engine s_shmy_audio_engine;

int create_context() {
    ma_result result;

    // The order in the array defines the order of priority. The vtables will be passed in to the resource manager config.
    ma_decoding_backend_vtable* pCustomBackendVTables[] = {
        ma_decoding_backend_libvorbis,
    };

    ma_resource_manager_config resourceManagerConfig;
    resourceManagerConfig = ma_resource_manager_config_init();
    resourceManagerConfig.ppCustomDecodingBackendVTables = pCustomBackendVTables;
    resourceManagerConfig.customDecodingBackendCount     = 1;
    resourceManagerConfig.pCustomDecodingBackendUserData = NULL;

    result = ma_resource_manager_init(&resourceManagerConfig, &s_shmy_audio_resources);
    if (result != MA_SUCCESS) {
        return result;
    }

    // Once we have a resource manager we can create the engine.
    ma_engine_config engineConfig = ma_engine_config_init();
    engineConfig.pResourceManager = &s_shmy_audio_resources;

    result = ma_engine_init(&engineConfig, &s_shmy_audio_engine);
    s_shmy_audio_engine.listenerCount = 1;

    return result;
}

void destroy_context() {
    ma_engine_uninit(&s_shmy_audio_engine);
    ma_resource_manager_uninit(&s_shmy_audio_resources);
}

ma_engine* get_audio_engine_ptr() {
    return &s_shmy_audio_engine;
}

} }
