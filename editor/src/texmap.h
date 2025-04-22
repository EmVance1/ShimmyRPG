#pragma once
#include <SFML/Graphics.hpp>
#include <unordered_map>


class TextureMap {
    static std::unordered_map<std::string, sf::Texture> textures;

public:
    static bool load_texture(const std::string& name, const std::string& filename) {
        return textures[name].loadFromFile(filename);
    }
    static const sf::Texture& get_texture(const std::string& name) {
        return textures[name];
    }
};

