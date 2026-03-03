#include "pch.h"
#include "validation.h"
#include "util/json.h"


#define PUSH_ERR(type, key, rec) result.errors.push_back({ SceneConfig::Error::type, key, rec })

#define VALIDATE(v, k, T, def, rec) (!v.HasMember(k) ? \
        (PUSH_ERR(WrongType, k, rec), def) : (!v[k].Is##T() ? \
        (PUSH_ERR(Missing, k, rec), def) : v[k]))

#define VALIDATE_T(v, k, T, def, rec) (!v.HasMember(k) ? \
        (PUSH_ERR(WrongType, k, rec), def) : (!v[k].Is##T() ? \
        (PUSH_ERR(Missing, k, rec), def) : v[k].Get##T()))

SceneConfig validate_scene_config(const rapidjson::Value& v) {
    auto result = SceneConfig{};

    if (!v.HasMember("label")) {
        PUSH_ERR(Missing, "label", true);
        result.label = "[]";
    } else if (!v["label"].IsString()) {
        PUSH_ERR(WrongType, "label", true);
        result.label = "[]";
    } else {
        result.label = v["label"].GetString();
    }

    if (!v.HasMember("zoom")) {
        result.zoom = 1.f;
    } else if (!v["label"].IsString()) {
        PUSH_ERR(WrongType, "zoom", true);
        result.zoom = 1.f;
    } else {
        result.zoom = v["zoom"].GetFloat();
    }

    if (!v.HasMember("scripts")) {
        PUSH_ERR(Missing, "scripts", true);
    } else if (!v["scripts"].IsArray()) {
        PUSH_ERR(WrongType, "scripts", true);
    } else {
        for (const auto& s : v["scripts"].GetArray()) {
            if (s.IsString()) {
                result.scripts.push_back(s.GetString());
            } else {
                PUSH_ERR(WrongType, "scripts[]", true);
            }
        }
    }

    if (!v.HasMember("tracks")) {
        PUSH_ERR(Missing, "tracks", true);
    } else if (!v["tracks"].IsArray()) {
        PUSH_ERR(WrongType, "tracks", true);
    } else {
        for (const auto& t : v["tracks"].GetArray()) {
            if (t.IsString()) {
                result.tracks.push_back(t.GetString());
            } else {
                PUSH_ERR(WrongType, "tracks[]", true);
            }
        }
    }

    if (!v.HasMember("portals")) {
        PUSH_ERR(Missing, "portals", true);
    } else if (!v["portals"].IsArray()) {
        PUSH_ERR(WrongType, "portals", true);
    } else {
        for (const auto& P : v["portals"].GetArray()) {
            auto p = PortalConfig{};

            if (!P.HasMember("id"))  { PUSH_ERR(Missing,   "id", true); continue; }
            if (!P["id"].IsString()) { PUSH_ERR(WrongType, "id", true); continue; }
            p.id = P["id"].GetString();

            if (!P.HasMember("position")) { PUSH_ERR(Missing,   "position", true); continue; }
            if (!P["position"].IsArray()) { PUSH_ERR(WrongType, "position", true); continue; }
            p.position = shmy::json::into_vector2f(P["position"]);

            if (!P.HasMember("exit"))  { PUSH_ERR(Missing,   "exit", true); continue; }
            if (!P["exit"].IsString()) { PUSH_ERR(WrongType, "exit", true); continue; }
            p.exit = P["exit"].GetString();

            if (P.HasMember("mouseover_radius")) {
                if (!P["mouseover_radius"].IsFloat()) { PUSH_ERR(WrongType, "mouseover_radius", true); continue; }
                p.preload_radius = P["preload_radius"].GetFloat();
            }
            if (P.HasMember("preload_radius")) {
                if (!P["preload_radius"].IsFloat()) { PUSH_ERR(WrongType, "preload_radius", true); continue; }
                p.preload_radius = P["preload_radius"].GetFloat();
            }
            if (P.HasMember("locked")) {
                if (!P["locked"].IsBool()) { PUSH_ERR(WrongType, "locked", true); continue; }
                p.locked = P["locked"].IsTrue();
            }

            result.portals.push_back(p);
        }
    }

    return result;
}

