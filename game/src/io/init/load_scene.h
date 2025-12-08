#pragma once
#include <rapidjson/document.h>
#include "world/area.h"


class SceneLoader {
private:
    Region* region;
    const rapidjson::Value& prefabs;

    Area* area;

private:
    void load_prefab(const rapidjson::Value& value, const std::string& name);
    void load_entity(const rapidjson::Value& value);

public:
    SceneLoader(Region* region, const rapidjson::Value& prefabs);

    void load(Area* area, const std::string& id);
    void load_gui();
};

