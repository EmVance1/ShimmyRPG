#include "pch.h"
#include "load_scene.h"
#include "world/game.h"
#include "world/scene.h"
#include "core/uuid.h"
#include "core/split.h"
#include "util/json.h"
#include "data/bundler.h"
#include <stdexcept>


static bool trait_playable(Entity& entity, const rapidjson::Value& stub, const rapidjson::Value& edef) {
    if (edef.HasMember("trait_playable")) {
        entity.set_playable(true);
        const auto& ply = JSON_GET(edef, "trait_playable");
        return JSON_IS_TRUE(ply, "current");
    } else if (stub.HasMember("trait_playable")) {
        entity.set_playable(true);
        const auto& ply = JSON_GET(stub, "trait_playable");
        return JSON_IS_TRUE(ply, "current");
    } else {
        return false;
    }
}
static void trait_movement(Entity& entity, const rapidjson::Value& stub, const rapidjson::Value& edef) {
    if (edef.HasMember("trait_movement")) {
        const auto& con = JSON_GET(stub, "trait_movement");
        entity.get_tracker().set_speed(JSON_GET_FLOAT(con, "speed"));
    } else if (stub.HasMember("trait_movement")) {
        const auto& con = JSON_GET(stub, "trait_movement");
        entity.get_tracker().set_speed(JSON_GET_FLOAT(con, "speed"));
    }
}
static void trait_speech(Entity& entity, const rapidjson::Value& stub, const rapidjson::Value& edef) {
    if (edef.HasMember("trait_speech")) {
        const auto& dia = JSON_GET(edef, "trait_speech");
        entity.get_actions().emplace_back(Entity::Action::Speak);
        entity.get_dialogue() = JSON_GET_STR(dia, "source");
    } else if (stub.HasMember("trait_speech")) {
        const auto& dia = JSON_GET(stub, "trait_speech");
        entity.get_actions().emplace_back(Entity::Action::Speak);
        entity.get_dialogue() = JSON_GET_STR(dia, "source");
    }
}
static void trait_examine(Entity& entity, const rapidjson::Value& stub, const rapidjson::Value& edef) {
    if (edef.HasMember("trait_examine")) {
        const auto& dia = JSON_GET(edef, "trait_examine");
        entity.get_actions().emplace_back(Entity::Action::Examine);
        entity.get_examination() = JSON_GET_STR(dia, "source");
    } else if (stub.HasMember("trait_examine")) {
        const auto& dia = JSON_GET(stub, "trait_examine");
        entity.get_actions().emplace_back(Entity::Action::Examine);
        entity.get_examination() = JSON_GET_STR(dia, "source");
    }
}
static void trait_customsort(Entity& entity, const rapidjson::Value& stub, const rapidjson::Value& edef) {
    if (edef.HasMember("trait_customsort")) {
        const auto& bry = JSON_GET_ARRAY(edef, "trait_customsort");
        entity.set_sorting_boundary(shmy::json::into_vector2f(bry[0]), shmy::json::into_vector2f(bry[1]));
    } else if (stub.HasMember("trait_customsort")) {
        const auto& bry = JSON_GET_ARRAY(stub, "trait_customsort");
        entity.set_sorting_boundary(shmy::json::into_vector2f(bry[0]), shmy::json::into_vector2f(bry[1]));
    }
}


#define HAS_TRAIT(t) (edef.HasMember(t) || stub.HasMember(t))

void SceneLoader::load_entity(const rapidjson::Value& edef, const std::string& src, EntityType type) {
    const auto& stub = (type == EntityType::Character) ? chars[src.c_str()] : props[src.c_str()];

    const auto tex_path = shmy::core::split(JSON_GET_STR(stub, "texture"), '.');
    if (!scene->refs.contains(tex_path[0])) {
        scene->refs.insert(tex_path[0]);
        shmy::data::Bundler::require(tex_path[0]);
    }
    const auto& bundle = shmy::data::Bundler::get_bundle(tex_path[0]);

    // BASIC INFO =============================================
    const auto id = (type == EntityType::Character) ? src : shmy::core::generate_uuid_v4();
    const auto name = edef.HasMember("name") ? JSON_GET_STR(edef, "name") : JSON_GET_STR(stub, "name");
    const auto handle = (uint32_t)game->m_entities.size();
    game->m_entity_map[id] = handle;
    auto& entity = game->m_entities.emplace_back(
        id, name,
        bundle.get_atlas(tex_path[1]),
        bundle.get_atlas(tex_path[1] + ".outline"),
        bundle.get_alphamap(tex_path[1]),
        (type == EntityType::Character) ? &scene->pathfinder : nullptr
    );

    // POSITION ===============================================
    const auto& pos = JSON_GET(edef, "position");
    if (pos.HasMember("world")) {
        entity.set_position(shmy::json::into_vector2f(pos["world"]), scene->world_to_screen);
    } else if (pos.HasMember("iso")) {
        entity.set_sprite_position(shmy::json::into_vector2f(pos["iso"]));
    }

    // TAGS ===================================================
    if (edef.HasMember("tags")) {
        for (const auto& tag : JSON_GET_ARRAY(edef, "tags")) {
            entity.get_tags().emplace(tag.GetString());
        }
    }
    if (stub.HasMember("tags")) {
        for (const auto& tag : JSON_GET_ARRAY(stub, "tags")) {
            entity.get_tags().emplace(tag.GetString());
        }
    }

    // TRAITS =================================================
    switch (type) {
    case EntityType::Character:
        trait_movement(entity, stub, edef);
        trait_speech(entity, stub, edef);
        if (trait_playable(entity, stub, edef)) {
            if (game->m_active_player == UINT32_MAX) {
                game->m_active_player = handle;
            } else {
                throw std::runtime_error("exactly one entity MUST be designated 'active = true'\n");
            }
        }
        [[fallthrough]];
    case EntityType::Prop:
        trait_examine(entity, stub, edef);
        trait_customsort(entity, stub, edef);
        entity.get_actions().emplace_back(Entity::Action::MoveTo);
        // if (HAS_TRAIT("trait_breakable"))        { entity.get_actions().emplace(AttackAction{}); }
        // if (HAS_TRAIT("trait_inventory"))        { entity.get_actions().emplace(OpenInvAction{}); }
        // if (HAS_TRAIT("trait_pickup"))           { entity.get_actions().emplace(PickUpAction{}); }
        // if (entity.get_tags().contains("door"))        { entity.get_actions().emplace(OpenDoorAction{}); }
        // if (entity.get_tags().contains("simple_lock")) { entity.get_actions().emplace(LockpickAction{}); }
    }

    if (edef.HasMember("is_offstage")) {
        entity.set_offstage(JSON_IS_TRUE(edef, "is_offstage"));
    }
    if (edef.HasMember("is_ghost")) {
        entity.set_ghost(JSON_IS_TRUE(edef, "is_ghost"));
    }

    scene->entities.push_back(handle);
}

