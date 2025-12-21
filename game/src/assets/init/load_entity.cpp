#include "pch.h"
#include "load_scene.h"
#include "world/scene.h"
#include "core/uuid.h"
#include "core/split.h"
#include "util/json.h"
#include "assets/manager.h"
#include <stdexcept>


void SceneLoader::load_entity(const rapidjson::Value& value) {
    // PREFABS
    if (value.HasMember("prop")) {
        load_from_prop(value, JSON_GET_STR(value, "prop"));
        return;
    }

    const auto tex_path = shmy::core::split(JSON_GET_STR(value, "texture"), '.');
    if (!scene->refs.contains(tex_path[0])) {
        scene->refs.insert(tex_path[0]);
        AssetManager::require(tex_path[0]);
    }
    const auto& bundle = AssetManager::get_bundle(tex_path[0]);

    const auto id = shmy::core::generate_uuid_v4();
    auto& entity = scene->entities.emplace(id, Entity{ id,
        JSON_GET_STR(value, "name"),
        bundle.get_atlas(tex_path[1]),
        bundle.get_atlas(tex_path[1] + "_outline"),
        bundle.get_alphamap(tex_path[1]),
        value.HasMember("trait_movement") ? &scene->pathfinder : nullptr
    }).first->second;

    // POSITION ===============================================
    const auto& pos = JSON_GET(value, "position");
    if (pos.HasMember("world")) {
        entity.set_position(shmy::json::into_vector2f(pos["world"]), scene->world_to_screen);
    } else if (pos.HasMember("iso")) {
        entity.set_sprite_position(shmy::json::into_vector2f(pos["iso"]));
    }

    // TAGS ===================================================
    for (const auto& tag : JSON_GET_ARRAY(value, "tags")) {
        entity.get_tags().emplace(tag.GetString());
    }

    // PLAYABLE ===============================================
    bool active = false;
    if (value.HasMember("trait_playable")) {
        // entity.set_playable(true);
        const auto& act = JSON_GET(value, "trait_playable");
        active = JSON_IS_TRUE(act, "active");
    }

    if (active) {
        if (scene->player_id == "") {
            scene->player_id = id;
        } else {
            throw std::runtime_error("exactly one entity MUST be designated 'active = true'\n");
        }
    } else {
        entity.get_actions().emplace(MoveToAction{ id });
        if (value.HasMember("trait_breakable"))        { entity.get_actions().emplace(AttackAction{}); }
        if (value.HasMember("trait_inventory"))        { entity.get_actions().emplace(OpenInvAction{}); }
        if (value.HasMember("trait_pickup"))           { entity.get_actions().emplace(PickUpAction{}); }
        if (entity.get_tags().contains("door"))        { entity.get_actions().emplace(OpenDoorAction{}); }
        if (entity.get_tags().contains("simple_lock")) { entity.get_actions().emplace(LockpickAction{}); }
    }

    // SCRIPTABLE =============================================
    if (value.HasMember("trait_scriptable")) {
        const auto& ids = JSON_GET(value, "trait_scriptable");
        const auto script = JSON_GET_STR(ids, "id");
        scene->script_to_uuid[script] = id;
        entity.set_script_id(script);
    }

    // MOVEMENT ===============================================
    if (value.HasMember("trait_movement")) {
        const auto& con = JSON_GET(value, "trait_movement");
        entity.get_tracker().set_speed(JSON_GET_FLOAT(con, "speed"));
    }

    // DIALOGUE ===============================================
    if (value.HasMember("trait_speech")) {
        const auto& dia = JSON_GET(value, "trait_speech");
        entity.get_actions().emplace(SpeakAction{ id, JSON_GET_STR(dia, "source") });
    }

    // EXAMINE ================================================
    if (value.HasMember("trait_examine")) {
        const auto& dia = JSON_GET(value, "trait_examine");
        entity.get_actions().emplace(ExamineAction{ id, JSON_GET_STR(dia, "source") });
    }

    // CUSTOM SORTING =========================================
    if (value.HasMember("trait_customsort")) {
        const auto& bry = JSON_GET_ARRAY(value, "trait_customsort");
        entity.set_sorting_boundary(shmy::json::into_vector2f(bry[0]), shmy::json::into_vector2f(bry[1]));
    }

    // RESTRICTIONS ===========================================
    if (value.HasMember("is_offstage")) {
        entity.set_offstage(JSON_IS_TRUE(value, "is_offstage"));
    }
    if (value.HasMember("is_ghost")) {
        entity.set_ghost(JSON_IS_TRUE(value, "is_ghost"));
    }
}


#define HAS_TRAIT(t) (value.HasMember(t) || prop.HasMember(t))

void SceneLoader::load_from_prop(const rapidjson::Value& value, const std::string& name) {
    const auto& prop = JSON_GET(props, name.c_str());

    const auto tex_path = shmy::core::split(JSON_GET_STR(prop, "texture"), '.');
    if (!scene->refs.contains(tex_path[0])) {
        scene->refs.insert(tex_path[0]);
        AssetManager::require(tex_path[0]);
    }
    const auto& bundle = AssetManager::get_bundle(tex_path[0]);

    const auto id = shmy::core::generate_uuid_v4();
    auto& entity = scene->entities.emplace(id, Entity{ id,
        JSON_GET_STR(prop, "name"),
        bundle.get_atlas(tex_path[1]),
        bundle.get_atlas(tex_path[1] + "_outline"),
        bundle.get_alphamap(tex_path[1]),
        HAS_TRAIT("trait_movement") ? &scene->pathfinder : nullptr
    }).first->second;

    // POSITION ===============================================
    const auto& pos = JSON_GET(value, "position");
    if (pos.HasMember("world")) {
        entity.set_position(shmy::json::into_vector2f(pos["world"]), scene->world_to_screen);
    } else if (pos.HasMember("iso")) {
        entity.set_sprite_position(shmy::json::into_vector2f(pos["iso"]));
    }

    // TAGS ===================================================
    if (prop.HasMember("tags")) {
        for (const auto& tag : JSON_GET_ARRAY(prop, "tags")) {
            entity.get_tags().emplace(tag.GetString());
        }
    }
    if (value.HasMember("tags")) {
        for (const auto& tag : JSON_GET_ARRAY(value, "tags")) {
            entity.get_tags().emplace(tag.GetString());
        }
    }
#ifdef SHMY_DEBUG
    if (!HAS_TRAIT("tags")) {
        throw std::invalid_argument("json object has no member 'tags'\n");
    }
#endif

    // PLAYABLE ===============================================
    bool active = false;
    if (prop.HasMember("trait_playable")) {
        // entity.set_playable(true);
        const auto& act = JSON_GET(prop, "trait_playable");
        active = JSON_IS_TRUE(act, "active");
    } else if (value.HasMember("trait_playable")) {
        // entity.set_playable(true);
        const auto& act = JSON_GET(value, "trait_playable");
        active = JSON_IS_TRUE(act, "active");
    }

    if (active) {
        if (scene->player_id == "") {
            scene->player_id = id;
        } else {
            throw std::runtime_error("exactly one entity MUST be designated 'active = true'\n");
        }
    } else {
        entity.get_actions().emplace(MoveToAction{ id });
        if (HAS_TRAIT("trait_breakable"))              { entity.get_actions().emplace(AttackAction{}); }
        if (HAS_TRAIT("trait_inventory"))              { entity.get_actions().emplace(OpenInvAction{}); }
        if (HAS_TRAIT("trait_pickup"))                 { entity.get_actions().emplace(PickUpAction{}); }
        if (entity.get_tags().contains("door"))        { entity.get_actions().emplace(OpenDoorAction{}); }
        if (entity.get_tags().contains("simple_lock")) { entity.get_actions().emplace(LockpickAction{}); }
    }

    // MOVEMENT ===============================================
    if (prop.HasMember("trait_movement")) {
        const auto& mov = JSON_GET(prop, "trait_movement");
        entity.get_tracker().set_speed(JSON_GET_FLOAT(mov, "speed"));
    } else if (value.HasMember("trait_movement")) {
        const auto& mov = JSON_GET(value, "trait_movement");
        entity.get_tracker().set_speed(JSON_GET_FLOAT(mov, "speed"));
    }

    // SCRIPTABLE =============================================
    if (prop.HasMember("trait_scriptable")) {
        const auto& ids = JSON_GET(prop, "trait_scriptable");
        const auto script = JSON_GET_STR(ids, "script_id");
        scene->script_to_uuid[script] = id;
        entity.set_script_id(script);
    } else if (value.HasMember("trait_scriptable")) {
        const auto& ids = JSON_GET(value, "trait_scriptable");
        const auto script = JSON_GET_STR(ids, "script_id");
        scene->script_to_uuid[script] = id;
        entity.set_script_id(script);
    }

    // DIALOGUE ===============================================
    if (prop.HasMember("trait_speech")) {
        const auto& dia = JSON_GET(prop, "trait_speech");
        entity.get_actions().emplace(SpeakAction{ id, JSON_GET_STR(dia, "source") });
    } else if (value.HasMember("trait_speech")) {
        const auto& dia = JSON_GET(value, "trait_speech");
        entity.get_actions().emplace(SpeakAction{ id, JSON_GET_STR(dia, "source") });
    }

    // EXAMINE ================================================
    if (prop.HasMember("trait_examine")) {
        const auto& dia = JSON_GET(prop, "trait_examine");
        entity.get_actions().emplace(ExamineAction{ id, JSON_GET_STR(dia, "source") });
    } else if (value.HasMember("trait_examine")) {
        const auto& dia = JSON_GET(value, "trait_examine");
        entity.get_actions().emplace(ExamineAction{ id, JSON_GET_STR(dia, "source") });
    }

    // CUSTOM SORTING =========================================
    if (prop.HasMember("trait_customsort")) {
        const auto& bry = JSON_GET_ARRAY(prop, "trait_customsort");
        entity.set_sorting_boundary(shmy::json::into_vector2f(bry[0]), shmy::json::into_vector2f(bry[1]));
    } else if (value.HasMember("trait_customsort")) {
        const auto& bry = JSON_GET_ARRAY(value, "trait_customsort");
        entity.set_sorting_boundary(shmy::json::into_vector2f(bry[0]), shmy::json::into_vector2f(bry[1]));
    }

    // RESTRICTIONS ===========================================
    if (value.HasMember("is_offstage")) {
        entity.set_offstage(JSON_IS_TRUE(value, "is_offstage"));
    }
    if (value.HasMember("is_ghost")) {
        entity.set_ghost(JSON_IS_TRUE(value, "is_ghost"));
    }
}

