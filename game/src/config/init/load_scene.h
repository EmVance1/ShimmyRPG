#pragma once
#include <rapidjson/document.h>
#include "world/scene.h"
#include <vector>


class SceneLoader {
private:
    Game* game;
    const rapidjson::Value& chars;
    const rapidjson::Value& props;

    Scene* scene;
    std::vector<size_t> portals;

    enum class EntityType {
        Character,
        Prop,
    };

private:
    void load_entity(const rapidjson::Value& edef, const std::string& src, EntityType type);

public:
    SceneLoader(Game* game, const rapidjson::Value& chars, const rapidjson::Value& props);

    void load(Scene* scene, const std::string& r_id, const std::string& s_id);

    const std::vector<size_t>& get_portals() const;
};

