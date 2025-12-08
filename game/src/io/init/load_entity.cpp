#include "pch.h"
#include "load_scene.h"
#include "world/area.h"
#include "world/region.h"
#include "core/uuid.h"
#include "util/json.h"
#include <stdexcept>


void SceneLoader::load_prefab(const rapidjson::Value& value, const std::string& name) {
    const auto& prefab = JSON_GET(prefabs, name.c_str());

    const auto e_id = shmy::core::generate_uuid_v4();
    const auto tex = JSON_GET_STR(prefab, "texture");

    area->entities[e_id] = Entity(e_id,
        region->m_atlases.at(tex),
        region->m_atlases.at(tex + std::string("_outline")),
        region->m_alphamaps.at(tex + std::string("_map")),
        &area->pathfinder, value.HasMember("trait_movement") || prefab.HasMember("trait_movement")
    );
    auto& entity = area->entities[e_id];

    // POSITION ===============================================
    const auto& pos = JSON_GET(value, "position");
    if (pos.HasMember("world")) {
        entity.set_position(shmy::json::into_vector2f(pos["world"]), area->cart_to_iso);
    } else if (pos.HasMember("iso")) {
        entity.set_sprite_position(shmy::json::into_vector2f(pos["iso"]));
    }

    // TAGS ===================================================
    if (prefab.HasMember("tags")) {
        for (const auto& tag : JSON_GET_ARRAY(prefab, "tags")) {
            entity.get_tags().emplace(tag.GetString());
        }
    } else if (value.HasMember("tags")) {
        for (const auto& tag : JSON_GET_ARRAY(value, "tags")) {
            entity.get_tags().emplace(tag.GetString());
        }
    }
#ifdef VANGO_DEBUG
    else {
        throw std::invalid_argument("json object has no member 'tags'\n");
    }
#endif

    if (entity.get_tags().contains("player")) {
        if (area->player_id == "") {
            area->player_id = e_id;
        } else {
            throw std::runtime_error("exactly one entity MUST be designated 'player'\n");
        }
    } else {
        entity.get_actions().emplace_back(MoveToAction{});
        entity.get_actions().emplace_back(AttackAction{});
        if (entity.get_tags().contains("npc"))         { entity.get_actions().emplace_back(SpeakAction{}); }
        if (entity.get_tags().contains("door"))        { entity.get_actions().emplace_back(OpenDoorAction{}); }
        if (entity.get_tags().contains("chest"))       { entity.get_actions().emplace_back(OpenInvAction{}); }
        if (entity.get_tags().contains("simple_lock")) { entity.get_actions().emplace_back(LockpickAction{}); }
        if (entity.get_tags().contains("carryable"))   { entity.get_actions().emplace_back(PickUpAction{}); }
    }

    // MOVEMENT ===============================================
    if (prefab.HasMember("trait_movement")) {
        const auto& con = JSON_GET(prefab, "trait_movement");
        entity.get_tracker().set_speed(JSON_GET_FLOAT(con, "speed"));
    } else if (value.HasMember("trait_movement")) {
        const auto& con = JSON_GET(value, "trait_movement");
        entity.get_tracker().set_speed(JSON_GET_FLOAT(con, "speed"));
    }

    // SCRIPTABLE =============================================
    if (prefab.HasMember("trait_scriptable")) {
        const auto& ids = JSON_GET(prefab, "trait_scriptable");
        const auto script = JSON_GET_STR(ids, "script_id");
        const auto story = JSON_GET_STR(ids, "story_id");
        area->script_to_uuid[script] = e_id;
        entity.set_script_id(script);
        entity.set_story_id(story);
    } else if (value.HasMember("trait_scriptable")) {
        const auto& ids = JSON_GET(value, "trait_scriptable");
        const auto script = JSON_GET_STR(ids, "script_id");
        const auto story = JSON_GET_STR(ids, "story_id");
        area->script_to_uuid[script] = e_id;
        entity.set_script_id(script);
        entity.set_story_id(story);
    }

    // DIALOGUE ===============================================
    if (prefab.HasMember("trait_speech")) {
        const auto& dia = JSON_GET(prefab, "trait_speech");
        if (dia.HasMember("file")) {
            entity.set_dialogue(JSON_GET_STR(dia, "file"));
        } else {
            entity.set_dialogue(JSON_GET_STR(dia, "line"));
        }
    } else if (value.HasMember("trait_speech")) {
        const auto& dia = JSON_GET(value, "trait_speech");
        if (dia.HasMember("file")) {
            entity.set_dialogue(JSON_GET_STR(dia, "file"));
        } else {
            entity.set_dialogue(JSON_GET_STR(dia, "line"));
        }
    }

    // CUSTOM SORTING =========================================
    if (prefab.HasMember("trait_customsort")) {
        const auto& bry = JSON_GET_ARRAY(prefab, "trait_customsort");
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

void SceneLoader::load_entity(const rapidjson::Value& value) {
    // PREFABS
    if (value.HasMember("prefab")) {
        load_prefab(value, JSON_GET_STR(value, "prefab"));
        return;
    }

    const auto e_id = shmy::core::generate_uuid_v4();
    const auto tex = JSON_GET_STR(value, "texture");
    area->entities[e_id] = Entity(e_id,
        region->m_atlases.at(tex),
        region->m_atlases.at(tex + std::string("_outline")),
        region->m_alphamaps.at(tex + std::string("_map")),
        &area->pathfinder, value.HasMember("trait_movement")
    );
    auto& entity = area->entities[e_id];

    // POSITION ===============================================
    const auto& pos = JSON_GET(value, "position");
    if (pos.HasMember("world")) {
        entity.set_position(shmy::json::into_vector2f(pos["world"]), area->cart_to_iso);
    } else if (pos.HasMember("iso")) {
        entity.set_sprite_position(shmy::json::into_vector2f(pos["iso"]));
    }

    // TAGS ===================================================
    for (const auto& tag : JSON_GET_ARRAY(value, "tags")) {
        entity.get_tags().emplace(tag.GetString());
    }
    if (entity.get_tags().contains("player")) {
        if (area->player_id == "") {
            area->player_id = e_id;
        } else {
            throw std::runtime_error("exactly one entity MUST be designated 'player'\n");
        }
    } else {
        entity.get_actions().emplace_back(MoveToAction{});
        entity.get_actions().emplace_back(AttackAction{});
        if (entity.get_tags().contains("npc"))         { entity.get_actions().emplace_back(SpeakAction{}); }
        if (entity.get_tags().contains("door"))        { entity.get_actions().emplace_back(OpenDoorAction{}); }
        if (entity.get_tags().contains("chest"))       { entity.get_actions().emplace_back(OpenInvAction{}); }
        if (entity.get_tags().contains("simple_lock")) { entity.get_actions().emplace_back(LockpickAction{}); }
        if (entity.get_tags().contains("carryable"))   { entity.get_actions().emplace_back(PickUpAction{}); }
    }

    // SCRIPTABLE =============================================
    if (value.HasMember("trait_scriptable")) {
        const auto& ids = JSON_GET(value, "trait_scriptable");
        const auto script = JSON_GET_STR(ids, "script_id");
        const auto story = JSON_GET_STR(ids, "story_id");
        area->script_to_uuid[script] = e_id;
        entity.set_script_id(script);
        entity.set_story_id(story);
    }

    // MOVEMENT ===============================================
    if (value.HasMember("trait_movement")) {
        const auto& con = JSON_GET(value, "trait_movement");
        entity.get_tracker().set_speed(JSON_GET_FLOAT(con, "speed"));
    }

    // DIALOGUE ===============================================
    if (value.HasMember("trait_speech")) {
        const auto& dia = JSON_GET(value, "trait_speech");
        if (dia.HasMember("file")) {
            entity.set_dialogue(JSON_GET_STR(dia, "file"));
        } else {
            entity.set_dialogue(JSON_GET_STR(dia, "line"));
        }
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

