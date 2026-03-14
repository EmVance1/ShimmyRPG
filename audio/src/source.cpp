#include "audio/source.h"
#include <miniaudio/miniaudio.h>
#include <miniaudio/extras/decoders/libvorbis/miniaudio_libvorbis.c>
#include "impl.h"
#ifdef VANGO_DEBUG
#include <iostream>
#endif


namespace shmy { namespace audio {

ma_engine* get_audio_engine_ptr();

static ma_decoder_config create_config() {
    static ma_decoding_backend_vtable* vtables[] = {
        ma_decoding_backend_libvorbis,
    };
    ma_decoder_config cfg = ma_decoder_config_init_default();
    cfg.ppCustomBackendVTables = vtables;
    cfg.customBackendCount     = 1;
    cfg.pCustomBackendUserData = NULL;
    return cfg;
}
static ma_decoder_config decoder_config = create_config();


#ifdef VANGO_DEBUG
#define MA_CHECK(x) do { result = x; if (result != MA_SUCCESS) { \
    std::cerr << "miniaudio error: " << ma_result_description(result) << "\n"; return {}; } } while (0)
#else
#define MA_CHECK(x) x
#endif

std::optional<Buffer> Buffer::load_file(const std::filesystem::path& path) {
    ma_decoder decoder{};
    ma_result result;

#ifdef _WIN32
    MA_CHECK(ma_decoder_init_file_w(path.c_str(), &decoder_config, &decoder));
#else
    MA_CHECK(ma_decoder_init_file(path.c_str(), &decoder_config, &decoder));
#endif

    auto res = Buffer{};
    res.sample_rate = decoder.outputSampleRate;
    res.channels    = decoder.outputChannels;
    res.format      = decoder.outputFormat;

    const size_t chunk_frames = 4096; // arbitrary
    const size_t frame_size   = ma_get_bytes_per_sample(decoder.outputFormat) * decoder.outputChannels;
    const size_t chunk_size   = chunk_frames * frame_size;
    size_t total_frames_read = 0;

    while (true) {
        res.pcm.resize(total_frames_read * frame_size + chunk_size);
        size_t frames_read = 0;
        MA_CHECK(ma_decoder_read_pcm_frames(
            &decoder,
            res.pcm.data() + total_frames_read * frame_size,
            chunk_frames,
            (ma_uint64*)&frames_read
        ));
        total_frames_read += frames_read;
        if (frames_read < chunk_frames) break;
    }

    res.frame_count = total_frames_read;
    res.pcm.resize(total_frames_read * frame_size);
    ma_decoder_uninit(&decoder);
    return res;
}


Stream::Stream(Stream&& other) noexcept
    : m_impl(other.m_impl)
{
    other.m_impl = nullptr;
}

Stream::~Stream() {
    if (m_impl) {
        ma_decoder_uninit(&m_impl->decoder);
        delete m_impl;
    }
}

Stream& Stream::operator=(Stream&& other) noexcept {
    this->~Stream();
    m_impl = other.m_impl;
    other.m_impl = nullptr;
    return *this;
}

std::optional<Stream> Stream::open_file(const std::filesystem::path& path) {
    auto res = Stream{};
    res.m_impl = new ImplT();
    ma_result result = MA_SUCCESS;

#ifdef _WIN32
    MA_CHECK(ma_decoder_init_file_w(path.c_str(), &decoder_config, &res.m_impl->decoder));
#else
    MA_CHECK(ma_decoder_init_file(path.c_str(), &decoder_config, &res.m_impl->decoder));
#endif

    if (result == MA_SUCCESS) {
        return res;
    } else {
#ifdef VANGO_DEBUG
        std::cerr << "miniaudio internal error: " << ma_result_description(result) << "\n";
#endif
        return {};
    }
}


} }

