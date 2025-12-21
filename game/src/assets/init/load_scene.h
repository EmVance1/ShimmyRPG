#pragma once
#include <rapidjson/document.h>
#include "world/scene.h"
#include <vector>


class SceneLoader {
private:
    Game* game;
    const rapidjson::Value& props;

    Scene* scene;
    std::vector<size_t> portals;

private:
    void load_from_prop(const rapidjson::Value& value, const std::string& name);
    void load_entity(const rapidjson::Value& value);

public:
    SceneLoader(Game* game, const rapidjson::Value& prefabs);

    void load(Scene* scene, const std::string& r_id, const std::string& s_id);
    void load_gui();

    const std::vector<size_t>& get_portals() const;
};

