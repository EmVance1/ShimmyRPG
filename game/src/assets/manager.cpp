#include "pch.h"
#include "manager.h"
#include "graphics/filters.h"
#include "core/split.h"
#include "util/json.h"


std::unordered_map<std::string, AssetManager::BundleDef> AssetManager::manifest;
std::unordered_map<std::string, AssetManager::Bundle> AssetManager::store;
std::unordered_map<std::string, AssetManager::Bundle> AssetManager::cache;
sf::Texture AssetManager::empty_tex(sf::Image({ 1, 1 }, sf::Color::White));



AssetManager::Bundle::Bundle(Bundle&& other) noexcept
    : refcount(other.refcount),
    LRUindex(other.LRUindex),
    textures(std::move(other.textures)),
    atlases(std::move(other.atlases)),
    alphamaps(std::move(other.alphamaps))
{}
AssetManager::Bundle& AssetManager::Bundle::operator=(Bundle&& other) noexcept {
    refcount = other.refcount;
    LRUindex = other.LRUindex;
    textures = std::move(other.textures);
    textures  = std::move(other.textures);
    atlases   = std::move(other.atlases);
    alphamaps = std::move(other.alphamaps);
    return *this;
}

AssetManager::Bundle AssetManager::Bundle::load(const BundleDef& def) {
    constexpr int OUTLINE_WIDTH = 5;
    auto result = Bundle();

    for (const auto& [name, v] : def) {
        const auto img = sf::Image(v.file);
        std::ignore = result.atlases[name].loadFromImage(img, v.dims);
        result.atlases[name].setSmooth(v.smooth);
        if (v.outline) {
            std::ignore = result.atlases[name + "_outline"].loadFromImage(
                (img.getSize().x < 200) ? shmy::filter::outline(img, OUTLINE_WIDTH) : shmy::filter::outline_threaded(img, OUTLINE_WIDTH), v.dims
            );
            result.atlases[name + "_outline"].setSmooth(v.smooth);
        }
        if (v.click) {
            result.alphamaps[name].loadFromImage(
                (img.getSize().x < 200) ? shmy::filter::clickmap(img, OUTLINE_WIDTH) : shmy::filter::clickmap_threaded(img, OUTLINE_WIDTH)
            );
        }
    }

    return result;
}


const sf::Texture& AssetManager::Bundle::get_texture(const std::string& name) const {
    return textures.at(name);
}

const sfu::TextureAtlas& AssetManager::Bundle::get_atlas(const std::string& name) const {
    return atlases.at(name);
}

const sfu::AlphaMap& AssetManager::Bundle::get_alphamap(const std::string& name) const {
    return alphamaps.at(name);
}

// size_t AssetManager::Bundle::sizeof_textures() {
//     return 0;
// }


void AssetManager::cache_insert(const std::string& name, Bundle&& bundle) {
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

AssetManager::Bundle AssetManager::cache_remove(const std::string& name) {
    auto temp = std::move(cache[name]);
    cache.erase(name);
    for (auto& [k, v] : cache) {
        if (v.LRUindex > temp.LRUindex) {
            v.LRUindex--;
        }
    }
    return temp;
}


void AssetManager::init(const std::filesystem::path& path) {
    const auto img_types = std::unordered_set<std::string>{ ".png", ".jpg", ".jpeg" };
    const auto aud_types = std::unordered_set<std::string>{ ".wav", ".ogg" };

    for (const auto& dir : std::fs::directory_iterator(path)) {
        if (!dir.is_directory()) continue;

        const auto bundle = dir.path().stem().string();
        const auto specs  = shmy::json::load_from_file(dir.path() / "bundle.json");
        auto& def = manifest.emplace(bundle, AssetManager::BundleDef{}).first->second;

        for (const auto& f : std::fs::directory_iterator(dir)) {
            const auto ext = f.path().extension().string();
            if (img_types.contains(ext)) {
                const auto  name = f.path().stem().string();
                const auto& fdef = JSON_GET(specs, name.c_str());
                def[name] = AssetManager::TextureDef{
                    .file = f.path(),
                    .dims = shmy::json::into_vector2u(JSON_GET(fdef, "dims")),
                    .outline = JSON_IS_TRUE(fdef, "outline"),
                    .smooth  = JSON_IS_TRUE(fdef, "smooth"),
                    .click   = JSON_IS_TRUE(fdef, "click"),
                };
            }
        }

        manifest[bundle] = def;
    }
}

void AssetManager::set_caching_on(bool _cache) {
    caching_on = _cache;
    if (!caching_on) {
        cache.clear();
    }
}

void AssetManager::reset() {
    caching_on = false;
    manifest.clear();
    store.clear();
    cache.clear();
}


void AssetManager::prepare(const std::string& bundle) {
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

void AssetManager::require(const std::string& bundle) {
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

void AssetManager::release(const std::string& bundle) {
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


const AssetManager::Bundle& AssetManager::get_bundle(const std::string& bundle) {
    return store.at(bundle);
}

const sf::Texture& AssetManager::get_texture(const std::string& path) {
    const auto parts = shmy::core::split(path, '.');
    return store[parts[0]].get_texture(parts[1]);
}

const sfu::TextureAtlas& AssetManager::get_atlas(const std::string& path) {
    const auto parts = shmy::core::split(path, '.');
    return store[parts[0]].get_atlas(parts[1]);
}

const sfu::AlphaMap& AssetManager::get_alphamap(const std::string& path) {
    const auto parts = shmy::core::split(path, '.');
    return store[parts[0]].get_alphamap(parts[1]);
}

const sf::Texture& AssetManager::nulltex() {
    return empty_tex;
}
