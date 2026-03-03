#include "pch.h"
#include "bundler.h"
#include "graphics/filters.h"
#include "core/split.h"
#include "util/json.h"


namespace shmy::data {

std::unordered_map<std::string, Bundler::BundleDef> Bundler::manifest;
std::unordered_map<std::string, Bundler::Bundle> Bundler::store;
std::unordered_map<std::string, Bundler::Bundle> Bundler::cache;
sf::Texture Bundler::empty_tex(sf::Image({ 1, 1 }, sf::Color::White));
sfu::TextureAtlas Bundler::empty_sheet(sf::Image({ 1, 1 }, sf::Color::White), { 1, 1 });



Bundler::Bundle::Bundle(Bundle&& other) noexcept
    : refcount(other.refcount),
    LRUindex(other.LRUindex),
    textures(std::move(other.textures)),
    atlases(std::move(other.atlases)),
    alphamaps(std::move(other.alphamaps)),
    sounds(std::move(other.sounds)),
    streams(std::move(other.streams)),
    shaders(std::move(other.shaders))
{}
Bundler::Bundle& Bundler::Bundle::operator=(Bundle&& other) noexcept {
    refcount = other.refcount;
    LRUindex = other.LRUindex;
    textures  = std::move(other.textures);
    atlases   = std::move(other.atlases);
    alphamaps = std::move(other.alphamaps);
    sounds    = std::move(other.sounds);
    streams   = std::move(other.streams);
    shaders   = std::move(other.shaders);
    return *this;
}


Bundler::Bundle Bundler::Bundle::load(const BundleDef& def) {
    constexpr int OUTLINE_WIDTH = 5;
    auto result = Bundle();

    for (const auto& [name, v] : def.textures) {
        const auto img = sf::Image(v.file);
        std::ignore = result.atlases[name].loadFromImage(img, v.dims);
        result.atlases[name].setSmooth(v.smooth);
        if (v.outline) {
            std::ignore = result.atlases[name + ".outline"].loadFromImage(
                (img.getSize().x < 200) ? shmy::filter::outline(img, OUTLINE_WIDTH) : shmy::filter::outline_threaded(img, OUTLINE_WIDTH), v.dims
            );
            result.atlases[name + ".outline"].setSmooth(v.smooth);
        }
        if (v.click) {
            result.alphamaps[name].loadFromImage(
                (img.getSize().x < 200) ? shmy::filter::clickmap(img, OUTLINE_WIDTH) : shmy::filter::clickmap_threaded(img, OUTLINE_WIDTH)
            );
        }
    }

    for (const auto& [name, v] : def.sounds) {
        if (v.stream) {
            result.streams[name] = *shmy::audio::Stream::open_file(v.file);
        } else {
            result.sounds[name] = *shmy::audio::Buffer::load_file(v.file);
        }
    }

    for (const auto& [name, v] : def.shaders) {
        std::ignore = result.shaders[name].loadFromFile(v.file, v.type);
        auto& s = result.shaders[name];
        for (const auto& [k, U] : v.uniforms) {
            std::visit([&](auto&& u){ s.setUniform(k, u); }, U);
        }
    }

    return result;
}


const sf::Texture& Bundler::Bundle::get_texture(const std::string& name) const {
    return textures.at(name);
}

const sfu::TextureAtlas& Bundler::Bundle::get_atlas(const std::string& name) const {
    return atlases.at(name);
}

const sfu::AlphaMap& Bundler::Bundle::get_alphamap(const std::string& name) const {
    return alphamaps.at(name);
}

const shmy::audio::Buffer& Bundler::Bundle::get_sound(const std::string& name) const {
    return sounds.at(name);
}

shmy::audio::Stream& Bundler::Bundle::get_stream(const std::string& name) {
    return streams.at(name);
}

const sf::Shader& Bundler::Bundle::get_shader(const std::string& name) const {
    return shaders.at(name);
}

// size_t Bundler::Bundle::sizeof_textures() const {
//     return 0;
// }


const std::unordered_map<std::string, sf::Texture>& Bundler::Bundle::get_textures() const {
    return textures;
}

const std::unordered_map<std::string, sfu::TextureAtlas>& Bundler::Bundle::get_atlases() const {
    return atlases;
}

const std::unordered_map<std::string, sfu::AlphaMap>& Bundler::Bundle::get_alphamaps() const {
    return alphamaps;
}

const std::unordered_map<std::string, shmy::audio::Buffer>& Bundler::Bundle::get_sounds() const {
    return sounds;
}

const std::unordered_map<std::string, shmy::audio::Stream>& Bundler::Bundle::get_streams() const {
    return streams;
}

const std::unordered_map<std::string, sf::Shader>& Bundler::Bundle::get_shaders() const {
    return shaders;
}


void Bundler::cache_insert(const std::string& name, Bundle&& bundle) {
    if (cache.size() == MAX_CACHE) {
        std::string victim;
        for (auto& [k, v] : cache) {
            if (v.LRUindex == MAX_CACHE-1) {
                victim = k;
                break;
            }
        }
        cache.erase(victim);
    }
    for (auto& [k, v] : cache) {
        v.LRUindex++;
    }
    cache[name] = std::move(bundle);
    cache[name].LRUindex = 0;
}

Bundler::Bundle Bundler::cache_remove(const std::string& name) {
    auto temp = std::move(cache[name]);
    cache.erase(name);
    for (auto& [k, v] : cache) {
        if (v.LRUindex > temp.LRUindex) {
            v.LRUindex--;
        }
    }
    return temp;
}


void Bundler::init(const std::filesystem::path& path, const sf::Vector2i& resolution) {
    const auto img_types = std::unordered_set<std::string>{ ".png", ".jpg", ".jpeg" };
    const auto aud_types = std::unordered_set<std::string>{ ".wav", ".ogg" };
    const auto sdr_types = std::unordered_set<std::string>{ ".vert", ".frag" };

    for (const auto& dir : std::fs::directory_iterator(path)) {
        if (!dir.is_directory()) continue;

        const auto bundle = dir.path().stem().string();
        const auto specs  = shmy::json::load_from_file(dir.path() / "bundle.json");
        auto& def = manifest.emplace(bundle, Bundler::BundleDef{}).first->second;

        for (const auto& f : std::fs::directory_iterator(dir)) {
            const auto ext = f.path().extension().string();
            if (img_types.contains(ext)) {
                const auto  name = f.path().stem().string();
                if (!specs.HasMember(name.c_str())) continue;
                const auto& fdef = JSON_GET(specs, name.c_str());
                if (fdef.IsNull()) continue;
                def.textures[name] = Bundler::TextureDef{
                    .file = f.path(),
                    .dims = shmy::json::into_vector2u(JSON_GET(fdef, "dims")),
                    .outline = JSON_IS_TRUE(fdef, "outline"),
                    .smooth  = JSON_IS_TRUE(fdef, "smooth"),
                    .click   = JSON_IS_TRUE(fdef, "click"),
                };
            }
            if (aud_types.contains(ext)) {
                const auto  name = f.path().stem().string();
                if (!specs.HasMember(name.c_str())) continue;
                const auto& fdef = JSON_GET(specs, name.c_str());
                if (fdef.IsNull()) continue;
                def.sounds[name] = Bundler::SoundDef{
                    .file = f.path(),
                    .stream  = JSON_IS_TRUE(fdef, "stream"),
                };
            }
            if (sdr_types.contains(ext)) {
                const auto  name = f.path().stem().string();
                if (!specs.HasMember(name.c_str())) continue;
                const auto& fdef = JSON_GET(specs, name.c_str());
                if (fdef.IsNull()) continue;
                def.shaders[name] = Bundler::ShaderDef{
                    .file = f.path(),
                    .type = (ext == ".vert" ? sf::Shader::Type::Vertex : sf::Shader::Type::Fragment),
                    .uniforms = {},
                };
                for (const auto& [k, u] : fdef.GetObject()) {
                    if (u.IsNull()) {
                        if (std::string(k.GetString()) == "u_resolution") {
                            def.shaders[name].uniforms[k.GetString()] = (sf::Glsl::Vec2)resolution;
                        } else if (std::string(k.GetString()) == "u_texture") {
                            def.shaders[name].uniforms[k.GetString()] = sf::Shader::CurrentTexture;
                        } else {
                            std::cout << "unknown builtin uniform\n";
                        }
                    } else if (u.IsArray()) {
                        if (u[0].IsFloat()) {
                            // std::cout << k.GetString() << ": u_fvec\n";
                            switch (u.GetArray().Size()) {
                            case 2:
                                def.shaders[name].uniforms[k.GetString()] = sf::Glsl::Vec2(
                                    u[0].GetFloat(),
                                    u[1].GetFloat()
                                );
                                break;
                            case 3:
                                def.shaders[name].uniforms[k.GetString()] = sf::Glsl::Vec3(
                                    u[0].GetFloat(),
                                    u[1].GetFloat(),
                                    u[2].GetFloat()
                                );
                                break;
                            case 4:
                                def.shaders[name].uniforms[k.GetString()] = sf::Glsl::Vec4(
                                    u[0].GetFloat(),
                                    u[1].GetFloat(),
                                    u[2].GetFloat(),
                                    u[3].GetFloat()
                                );
                                break;
                            }
                        } else if (u[0].IsInt()) {
                            // std::cout << k.GetString() << ": u_ivec\n";
                            switch (u.GetArray().Size()) {
                            case 2:
                                def.shaders[name].uniforms[k.GetString()] = sf::Glsl::Ivec2(
                                    u[0].GetInt(),
                                    u[1].GetInt()
                                );
                                break;
                            case 3:
                                def.shaders[name].uniforms[k.GetString()] = sf::Glsl::Ivec3(
                                    u[0].GetInt(),
                                    u[1].GetInt(),
                                    u[2].GetInt()
                                );
                                break;
                            case 4:
                                def.shaders[name].uniforms[k.GetString()] = sf::Glsl::Ivec4(
                                    u[0].GetInt(),
                                    u[1].GetInt(),
                                    u[2].GetInt(),
                                    u[3].GetInt()
                                );
                                break;
                            }
                        }
                    } else if (u.IsFloat()) {
                        // std::cout << k.GetString() << ": u_fscalar\n";
                        def.shaders[name].uniforms[k.GetString()] = u.GetFloat();
                    } else if (u.IsInt()) {
                        // std::cout << k.GetString() << ": u_iscalar\n";
                        def.shaders[name].uniforms[k.GetString()] = u.GetInt();
                    }
                }
            }
        }

        manifest[bundle] = def;
    }
}

void Bundler::reset() {
    caching_on = false;
    manifest.clear();
    store.clear();
    cache.clear();
}

void Bundler::set_caching_on(bool _cache) {
    caching_on = _cache;
    if (!caching_on) {
        cache.clear();
    }
}


void Bundler::prepare(const std::string& bundle) {
    /*
    if (store.contains(bundle)) {
        store[bundle].refcount++;
    } else if (caching_on && cache.contains(bundle)) {
        store[bundle] = cache_remove(bundle);
        store[bundle].refcount++;
    } else {
        store[bundle] = Bundle::load(bundle);
    }
    */
}

void Bundler::require(const std::string& bundle) {
    if (store.contains(bundle)) {
        store[bundle].refcount++;
    } else if (caching_on && cache.contains(bundle)) {
        store[bundle] = cache_remove(bundle);
        store[bundle].refcount++;
    } else {
        store[bundle] = Bundle::load(manifest[bundle]);
        store[bundle].refcount++;
    }
}

void Bundler::release(const std::string& bundle) {
    assert(store.contains(bundle));
    store[bundle].refcount--;
    if (store[bundle].refcount != 0) {
        return;
    }
    if (caching_on) {
        cache_insert(bundle, std::move(store[bundle]));
    }
    store.erase(bundle);
}


const Bundler::Bundle& Bundler::get_bundle(const std::string& bundle) {
    return store.at(bundle);
}

const sf::Texture& Bundler::get_texture(const std::string& path) {
    const auto parts = shmy::core::split(path, '.');
    return store[parts[0]].get_texture(parts[1]);
}

const sfu::TextureAtlas& Bundler::get_atlas(const std::string& path) {
    const auto parts = shmy::core::split(path, '.');
    return store[parts[0]].get_atlas(parts[1]);
}

const sfu::AlphaMap& Bundler::get_alphamap(const std::string& path) {
    const auto parts = shmy::core::split(path, '.');
    return store[parts[0]].get_alphamap(parts[1]);
}

const shmy::audio::Buffer& Bundler::get_sound(const std::string& path) {
    const auto parts = shmy::core::split(path, '.');
    return store[parts[0]].get_sound(parts[1]);
}

shmy::audio::Stream& Bundler::get_stream(const std::string& path) {
    const auto parts = shmy::core::split(path, '.');
    return store[parts[0]].get_stream(parts[1]);
}

const sf::Shader& Bundler::get_shader(const std::string& path) {
    const auto parts = shmy::core::split(path, '.');
    return store[parts[0]].get_shader(parts[1]);
}

const sf::Texture& Bundler::nulltex() {
    return empty_tex;
}

const sfu::TextureAtlas& Bundler::nullsheet() {
    return empty_sheet;
}

}
