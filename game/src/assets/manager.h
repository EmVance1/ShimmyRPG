#include <SFML/Graphics.hpp>
#include <sfutil/alphamap.h>
#include <sfutil/atlas.h>
#include <unordered_map>
#include <string>
#include <filesystem>


class AssetManager {
public:
    struct TextureDef {
        std::filesystem::path file;
        sf::Vector2u dims;
        bool outline;
        bool smooth;
        bool click;
    };
    using BundleDef = std::unordered_map<std::string, TextureDef>;

private:
    class Bundle {
    private:
        int refcount = 0;
        int LRUindex = 0;
        std::unordered_map<std::string, sf::Texture> textures;
        std::unordered_map<std::string, sfu::TextureAtlas> atlases;
        std::unordered_map<std::string, sfu::AlphaMap> alphamaps;

        Bundle(Bundle&& other) noexcept;
        Bundle& operator=(Bundle&& other) noexcept;

        static Bundle load(const BundleDef& def);

    public:
        Bundle() = default;
        Bundle(const Bundle&) = delete;

        const sf::Texture& get_texture(const std::string& name) const;
        const sfu::TextureAtlas& get_atlas(const std::string& name) const;
        const sfu::AlphaMap& get_alphamap(const std::string& name) const;

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
    static const sf::Texture& nulltex();
};

