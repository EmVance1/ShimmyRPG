#include "pch.h"
#include "world/area.h"
#include "world/region.h"
#include "util/json.h"
#include "util/uuid.h"


void Area::load_prefab(const rapidjson::Value& prefabs, const rapidjson::Value& value, const std::string& name) {
    const auto& prefab = prefabs.GetObject()[name.c_str()];

    const auto e_id = Uuid::generate_v4();
    const auto tex = prefab.GetObject()["texture"].GetString();

    entities[e_id] = Entity(e_id,
        p_region->m_atlases.at(tex),
        p_region->m_atlases.at(tex + std::string("_outline")),
        p_region->m_alphamaps.at(tex + std::string("_map")),
        &pathfinder, value.GetObject().HasMember("controller") || prefab.GetObject().HasMember("controller")
    );
    auto& entity = entities[e_id];

    // POSITION
    const auto pos = value.GetObject()["position"].GetObject();
    if (pos.HasMember("world")) {
        entity.set_position(json_to_vector2f(pos["world"]), cart_to_iso);
    } else if (pos.HasMember("iso")) {
        entity.set_sprite_position(json_to_vector2f(pos["iso"]));
    }

    // CONTROLLER
    if (prefab.GetObject().HasMember("controller")) {
        const auto con = prefab.GetObject()["controller"].GetObject();
        entity.get_tracker().set_speed(con["speed"].GetFloat());
    } else if (value.GetObject().HasMember("controller")) {
        const auto con = value.GetObject()["controller"].GetObject();
        entity.get_tracker().set_speed(con["speed"].GetFloat());
    }

    // BOUNDARY
    if (prefab.GetObject().HasMember("boundary")) {
        const auto bry = prefab.GetObject()["boundary"].GetArray();
        entity.set_sorting_boundary(json_to_vector2f(bry[0]), json_to_vector2f(bry[1]));
    } else if (value.GetObject().HasMember("boundary")) {
        const auto bry = value.GetObject()["boundary"].GetArray();
        entity.set_sorting_boundary(json_to_vector2f(bry[0]), json_to_vector2f(bry[1]));
    } else {
        entity.set_sorting_boundary(sf::Vector2f(0, 0));
    }

    // IDS
    if (prefab.GetObject().HasMember("ids")) {
        const auto ids = prefab.GetObject()["ids"].GetObject();
        script_name_LUT[ids["script"].GetString()] = e_id;
        story_name_LUT[ids["script"].GetString()] = ids["story"].GetString();
        entity.set_script_id(ids["script"].GetString());
    } else if (value.GetObject().HasMember("ids")) {
        const auto ids = value.GetObject()["ids"].GetObject();
        script_name_LUT[ids["script"].GetString()] = e_id;
        story_name_LUT[ids["script"].GetString()] = ids["story"].GetString();
        entity.set_script_id(ids["script"].GetString());
    }

    // DIALOGUE
    if (prefab.GetObject().HasMember("dialogue")) {
        const auto dia = prefab.GetObject()["dialogue"].GetObject();
        if (dia.HasMember("file")) {
            entity.set_dialogue(dia["file"].GetString());
        } else {
            entity.set_dialogue(dia["line"].GetString());
        }
    } else if (value.GetObject().HasMember("dialogue")) {
        const auto dia = value.GetObject()["dialogue"].GetObject();
        if (dia.HasMember("file")) {
            entity.set_dialogue(dia["file"].GetString());
        } else {
            entity.set_dialogue(dia["line"].GetString());
        }
    }

    // TAGS
    if (prefab.GetObject().HasMember("tags")) {
        for (const auto& tag : prefab.GetObject()["tags"].GetArray()) {
            entity.get_tags().emplace(tag.GetString());
        }
    }
    if (value.GetObject().HasMember("tags")) {
        for (const auto& tag : value.GetObject()["tags"].GetArray()) {
            entity.get_tags().emplace(tag.GetString());
        }
    }

    if (entity.get_tags().contains("player")) {
        if (player_id == "") {
            player_id = e_id;
        } else {
            std::cout << "error parsing entities: cannot have 2 player controllers\n";
            throw std::exception();
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
    if (value.GetObject().HasMember("prefab")) {
        load_prefab(prefabs, value, value.GetObject()["prefab"].GetString());
        return;
    }

    const auto e_id = Uuid::generate_v4();
    const auto tex = value.GetObject()["texture"].GetString();
    entities[e_id] = Entity(e_id,
        p_region->m_atlases.at(tex),
        p_region->m_atlases.at(tex + std::string("_outline")),
        p_region->m_alphamaps.at(tex + std::string("_map")),
        &pathfinder, value.GetObject().HasMember("controller")
    );
    auto& entity = entities[e_id];

    // POSITION
    const auto pos = value.GetObject()["position"].GetObject();
    if (pos.HasMember("world")) {
        entity.set_position(json_to_vector2f(pos["world"]), cart_to_iso);
    } else if (pos.HasMember("iso")) {
        entity.set_sprite_position(json_to_vector2f(pos["iso"]));
    }

    // CONTROLLER
    if (value.GetObject().HasMember("controller")) {
        const auto con = value.GetObject()["controller"].GetObject();
        entity.get_tracker().set_speed(con["speed"].GetFloat());
    }

    // BOUNDARY
    if (value.GetObject().HasMember("boundary")) {
        const auto bry = value.GetObject()["boundary"].GetArray();
        entity.set_sorting_boundary(json_to_vector2f(bry[0]), json_to_vector2f(bry[1]));
    } else {
        entity.set_sorting_boundary(sf::Vector2f(0, 0));
    }

    // IDS
    if (value.GetObject().HasMember("ids")) {
        const auto ids = value.GetObject()["ids"].GetObject();
        script_name_LUT[ids["script"].GetString()] = e_id;
        story_name_LUT[ids["script"].GetString()] = ids["story"].GetString();
        entity.set_script_id(ids["script"].GetString());
    }

    // DIALOGUE
    if (value.GetObject().HasMember("dialogue")) {
        const auto dia = value.GetObject()["dialogue"].GetObject();
        if (dia.HasMember("file")) {
            entity.set_dialogue(dia["file"].GetString());
        } else {
            entity.set_dialogue(dia["line"].GetString());
        }
    }

    // TAGS
    for (const auto& tag : value.GetObject()["tags"].GetArray()) {
        entity.get_tags().emplace(tag.GetString());
    }

    if (entity.get_tags().contains("player")) {
        if (player_id == "") {
            player_id = e_id;
        } else {
            std::cout << "error parsing entities: cannot have 2 player controllers\n";
            throw std::exception();
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

