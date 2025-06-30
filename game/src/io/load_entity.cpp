#include "pch.h"
#include "world/area.h"
#include "world/region.h"
#include "util/json.h"
#include "util/uuid.h"
#include "json_debug.h"
#include <stdexcept>


void Area::load_prefab(const rapidjson::Value& prefabs, const rapidjson::Value& value, const std::string& name) {
    const auto& prefab = JSON_GET(prefabs, name.c_str());

    const auto e_id = Uuid::generate_v4();
    const auto tex = JSON_GET_STR(prefab, "texture");

    entities[e_id] = Entity(e_id,
        p_region->m_atlases.at(tex),
        p_region->m_atlases.at(tex + std::string("_outline")),
        p_region->m_alphamaps.at(tex + std::string("_map")),
        &pathfinder, value.HasMember("controller") || prefab.HasMember("controller")
    );
    auto& entity = entities[e_id];

    // OFFSTAGE
    if (value.HasMember("offstage")) {
        entity.set_offstage(JSON_IS_TRUE(value, "offstage"));
    }

    // POSITION
    const auto& pos = JSON_GET(value, "position");
    if (pos.HasMember("world")) {
        entity.set_position(json_to_vector2f(pos["world"]), cart_to_iso);
    } else if (pos.HasMember("iso")) {
        entity.set_sprite_position(json_to_vector2f(pos["iso"]));
    }

    // CONTROLLER
    if (prefab.HasMember("controller")) {
        const auto& con = JSON_GET(prefab, "controller");
        entity.get_tracker().set_speed(JSON_GET_FLOAT(con, "speed"));
    } else if (value.HasMember("controller")) {
        const auto& con = JSON_GET(value, "controller");
        entity.get_tracker().set_speed(JSON_GET_FLOAT(con, "speed"));
    }

    // BOUNDARY
    if (prefab.HasMember("boundary")) {
        const auto& bry = JSON_GET_ARRAY(prefab, "boundary");
        entity.set_sorting_boundary(json_to_vector2f(bry[0]), json_to_vector2f(bry[1]));
    } else if (value.HasMember("boundary")) {
        const auto& bry = JSON_GET_ARRAY(value, "boundary");
        entity.set_sorting_boundary(json_to_vector2f(bry[0]), json_to_vector2f(bry[1]));
    } else {
        entity.set_sorting_boundary(sf::Vector2f(0, 0));
    }

    // IDS
    if (prefab.HasMember("ids")) {
        const auto& ids = JSON_GET(prefab, "ids");
        const auto script = JSON_GET_STR(ids, "script");
        script_name_LUT[script] = e_id;
        story_name_LUT[script] = JSON_GET_STR(ids, "story");
        entity.set_script_id(script);
    } else if (value.HasMember("ids")) {
        const auto& ids = JSON_GET(value, "ids");
        const auto script = JSON_GET_STR(ids, "script");
        script_name_LUT[script] = e_id;
        story_name_LUT[script] = JSON_GET_STR(ids, "story");
        entity.set_script_id(script);
    }

    // DIALOGUE
    if (prefab.HasMember("dialogue")) {
        const auto& dia = JSON_GET(prefab, "dialogue");
        if (dia.HasMember("file")) {
            entity.set_dialogue(JSON_GET_STR(dia, "file"));
        } else {
            entity.set_dialogue(JSON_GET_STR(dia, "line"));
        }
    } else if (value.HasMember("dialogue")) {
        const auto& dia = JSON_GET(value, "dialogue");
        if (dia.HasMember("file")) {
            entity.set_dialogue(JSON_GET_STR(dia, "file"));
        } else {
            entity.set_dialogue(JSON_GET_STR(dia, "line"));
        }
    }

    // TAGS
    if (prefab.HasMember("tags")) {
        for (const auto& tag : JSON_GET_ARRAY(prefab, "tags")) {
            entity.get_tags().emplace(tag.GetString());
        }
    } else if (value.HasMember("tags")) {
        for (const auto& tag : JSON_GET_ARRAY(value, "tags")) {
            entity.get_tags().emplace(tag.GetString());
        }
    } else {
        throw std::invalid_argument(std::string("invalid json access - object has no member 'tags'\n"));
    }

    if (entity.get_tags().contains("player")) {
        if (player_id == "") {
            player_id = e_id;
        } else {
            throw std::invalid_argument("exactly one entity MUST be designated 'player'\n");
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
}

void Area::load_entity(const rapidjson::Value& prefabs, const rapidjson::Value& value) {
    // PREFABS
    if (value.HasMember("prefab")) {
        load_prefab(prefabs, value, JSON_GET_STR(value, "prefab"));
        return;
    }

    const auto e_id = Uuid::generate_v4();
    const auto tex = JSON_GET_STR(value, "texture");
    entities[e_id] = Entity(e_id,
        p_region->m_atlases.at(tex),
        p_region->m_atlases.at(tex + std::string("_outline")),
        p_region->m_alphamaps.at(tex + std::string("_map")),
        &pathfinder, value.HasMember("controller")
    );
    auto& entity = entities[e_id];

    // OFFSTAGE
    if (value.HasMember("offstage")) {
        entity.set_offstage(JSON_IS_TRUE(value, "offstage"));
    }

    // POSITION
    const auto& pos = JSON_GET(value, "position");
    if (pos.HasMember("world")) {
        entity.set_position(json_to_vector2f(pos["world"]), cart_to_iso);
    } else if (pos.HasMember("iso")) {
        entity.set_sprite_position(json_to_vector2f(pos["iso"]));
    }

    // CONTROLLER
    if (value.HasMember("controller")) {
        const auto& con = JSON_GET(value, "controller");
        entity.get_tracker().set_speed(JSON_GET_FLOAT(con, "speed"));
    }

    // BOUNDARY
    if (value.HasMember("boundary")) {
        const auto& bry = JSON_GET_ARRAY(value, "boundary");
        entity.set_sorting_boundary(json_to_vector2f(bry[0]), json_to_vector2f(bry[1]));
    } else {
        entity.set_sorting_boundary(sf::Vector2f(0, 0));
    }

    // IDS
    if (value.HasMember("ids")) {
        const auto& ids = JSON_GET(value, "ids");
        const auto script = JSON_GET_STR(ids, "script");
        script_name_LUT[script] = e_id;
        story_name_LUT[script] = JSON_GET_STR(ids, "story");
        entity.set_script_id(script);
    }

    // DIALOGUE
    if (value.HasMember("dialogue")) {
        const auto& dia = JSON_GET(value, "dialogue");
        if (dia.HasMember("file")) {
            entity.set_dialogue(JSON_GET_STR(dia, "file"));
        } else {
            entity.set_dialogue(JSON_GET_STR(dia, "line"));
        }
    }

    // TAGS
    for (const auto& tag : JSON_GET_ARRAY(value, "tags")) {
        entity.get_tags().emplace(tag.GetString());
    }

    if (entity.get_tags().contains("player")) {
        if (player_id == "") {
            player_id = e_id;
        } else {
            throw std::invalid_argument("exactly one entity MUST be designated 'player'\n");
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
}

