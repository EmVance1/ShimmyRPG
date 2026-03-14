#pragma once
#include <filesystem>
#include <optional>
#include <vector>


namespace shmy { namespace audio {


class Buffer {
private:
    uint64_t frame_count = 0;
    uint32_t sample_rate = 0;
    uint32_t channels = 0;
    int format = 0;
    std::vector<uint8_t> pcm;

public:
    static std::optional<Buffer> load_file(const std::filesystem::path& path);

    friend class Player;
};


class Stream {
private:
    struct ImplT;
    ImplT* m_impl = nullptr;

public:
    Stream() = default;
    Stream(const Stream&) = delete;
    Stream(Stream&& other) noexcept;
    ~Stream();

    Stream& operator=(const Stream&) = delete;
    Stream& operator=(Stream&& other) noexcept;

    static std::optional<Stream> open_file(const std::filesystem::path& path);

    friend class Player;
};


} }

