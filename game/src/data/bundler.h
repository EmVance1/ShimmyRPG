#pragma once
#include <SFML/Graphics.hpp>
#include <sfutil/alphamap.h>
#include <sfutil/atlas.h>
#include <audio/lib.h>
#include <string>
#include <unordered_map>
#include <filesystem>


namespace shmy::data {

class Bundler {
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
    struct ShaderDef {
        using UniformT = std::variant<
            sf::Shader::CurrentTextureType,
            float,
            sf::Glsl::Vec2,
            sf::Glsl::Vec3,
            sf::Glsl::Vec4,
            int,
            sf::Glsl::Ivec2,
            sf::Glsl::Ivec3,
            sf::Glsl::Ivec4
        >;
        std::filesystem::path file;
        sf::Shader::Type type;
        std::unordered_map<std::string, UniformT> uniforms;
    };
    using TextureList = std::unordered_map<std::string, TextureDef>;
    using SoundList   = std::unordered_map<std::string, SoundDef>;
    using ShaderList  = std::unordered_map<std::string, ShaderDef>;
    struct BundleDef {
        TextureList textures;
        SoundList sounds;
        ShaderList shaders;
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
        std::unordered_map<std::string, sf::Shader> shaders;

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
        const sf::Shader& get_shader(const std::string& name) const;

        const std::unordered_map<std::string, sf::Texture>& get_textures() const;
        const std::unordered_map<std::string, sfu::TextureAtlas>& get_atlases() const;
        const std::unordered_map<std::string, sfu::AlphaMap>& get_alphamaps() const;
        const std::unordered_map<std::string, shmy::audio::Buffer>& get_sounds() const;
        const std::unordered_map<std::string, shmy::audio::Stream>& get_streams() const;
        const std::unordered_map<std::string, sf::Shader>& get_shaders() const;

        friend class Bundler;
    };

private:
    inline static constexpr int MAX_CACHE = 3;
    inline static bool caching_on = false;
    static std::unordered_map<std::string, BundleDef> manifest;
    static std::unordered_map<std::string, Bundle> store;
    static std::unordered_map<std::string, Bundle> cache;
    static sf::Texture empty_tex;
    static sfu::TextureAtlas empty_sheet;

private:
    static void cache_insert(const std::string& name, Bundle&& bundle);
    static Bundle cache_remove(const std::string& name);

public:
    static void init(const std::filesystem::path& path, const sf::Vector2i& resolution);
    static void reset();
    static void set_caching_on(bool cache);

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
    static const sf::Shader& get_shader(const std::string& path);
    static const sf::Texture& nulltex();
    static const sfu::TextureAtlas& nullsheet();
};

}
