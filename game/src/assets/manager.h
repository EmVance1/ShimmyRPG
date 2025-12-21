#include <SFML/Graphics.hpp>
#include <sfutil/alphamap.h>
#include <sfutil/atlas.h>
#include <audio/lib.h>
#include <unordered_map>
#include <string>
#include <filesystem>


class AssetManager {
private:
    struct TextureDef {
        std::filesystem::path file;
        sf::Vector2u dims;
        bool outline;
        bool smooth;
        bool click;
    };
    struct SoundDef {
        std::filesystem::path file;
        bool stream;
    };
    using TextureList = std::unordered_map<std::string, TextureDef>;
    using SoundList = std::unordered_map<std::string, SoundDef>;
    struct BundleDef {
        TextureList textures;
        SoundList sounds;
    };

    class Bundle {
    private:
        int refcount = 0;
        int LRUindex = 0;
        std::unordered_map<std::string, sf::Texture> textures;
        std::unordered_map<std::string, sfu::TextureAtlas> atlases;
        std::unordered_map<std::string, sfu::AlphaMap> alphamaps;
        std::unordered_map<std::string, shmy::audio::Buffer> sounds;
        std::unordered_map<std::string, shmy::audio::Stream> streams;

        Bundle(Bundle&& other) noexcept;
        Bundle& operator=(Bundle&& other) noexcept;

        static Bundle load(const BundleDef& def);

    public:
        Bundle() = default;
        Bundle(const Bundle&) = delete;

        const sf::Texture& get_texture(const std::string& name) const;
        const sfu::TextureAtlas& get_atlas(const std::string& name) const;
        const sfu::AlphaMap& get_alphamap(const std::string& name) const;
        const shmy::audio::Buffer& get_sound(const std::string& name) const;
        shmy::audio::Stream& get_stream(const std::string& name);

        friend class AssetManager;
    };

private:
    inline static constexpr int MAX_CACHE = 3;
    inline static bool caching_on = false;
    static std::unordered_map<std::string, BundleDef> manifest;
    static std::unordered_map<std::string, Bundle> store;
    static std::unordered_map<std::string, Bundle> cache;
    static sf::Texture empty_tex;

private:
    static void cache_insert(const std::string& name, Bundle&& bundle);
    static Bundle cache_remove(const std::string& name);

public:
    static void init(const std::filesystem::path& path);
    static void set_caching_on(bool cache);
    static void reset();

    // NOT THREADSAFE
    static void prepare(const std::string& bundle);
    static void require(const std::string& bundle);
    static void release(const std::string& bundle);

    static const Bundle& get_bundle(const std::string& bundle);
    static const sf::Texture& get_texture(const std::string& path);
    static const sfu::TextureAtlas& get_atlas(const std::string& path);
    static const sfu::AlphaMap& get_alphamap(const std::string& path);
    static const shmy::audio::Buffer& get_sound(const std::string& path);
    static shmy::audio::Stream& get_stream(const std::string& path);
    static const sf::Texture& nulltex();
};

