
Position = {
    WORLD = 0,
    ISO = 1,

    world = function(pos) return { x=pos[1], y=pos[2], mode=Position.WORLD } end,
    iso =   function(pos) return { x=pos[1], y=pos[2], mode=Position.ISO } end
}


function Setup(scene)
    scene.label = "Shimmy's Shack"
    scene.topleft = { 948, 315 }
    scene.scale = 5.35

    scene.background = {
        { "textures/ademmar/shimmyshack.png", { -68, -70, 2054, 1156 } }
    },

    -- front door
    scene:trigger_rect({ 265, 535, 120, 30 }, Action.goto{ index = 1, spawnpos = { 810, 360 }, suppress_triggers = true })
        :condition("!Ademmar_CurfewInPlace")

    -- welcome cutscene
    scene:trigger_rect({ 140, 0, 394, 380 },  Action.script("speech/ShimmyIntroScene.lua"))
        :condition("ShimmyIntroScene_IsActive & once")

    -- curfew blocker
    scene:trigger_rect({ 265, 535, 120, 30 }, Action.popup("There is a curfew in place. It is unwise to go outside."))
        :condition("Ademmar_CurfewInPlace")

    -- behind bar
    scene:trigger_rect({ 192, 0, 342, 100 },  Action.dialogue("speech/ShimmyBehindBar.shmy"))


    scene:entity("player_placeholder", Position.world{ 320, 480 }, { "player" })
        :trait_scriptable{ script_id = "player", story_id = "Stanza" }
        :trait_movement{ speed = 2.5 }

    scene:entity("anim_test", Position.world{ 330, 50 }, { "npc", "trader", "shimmy_staff" })
        :trait_scriptable{ script_id = "ShimmyPrime", story_id = "Shimmy" }
        :trait_movement{ speed = 2.0 }
        :trait_speech{ file = "speech/ShimmyDialogue.shmy" }

    scene:entity("npc_friendly_placeholder", Position.world{ 500, 290 }, { "npc", "shimmy_customer" })
        :trait_scriptable{ script_id = "BrianPrime", story_id = "Brian" }
        :trait_movement{ speed = 2.0 }
        :trait_speech{ file = "speech/BrianDialogue.shmy" }

    scene:entity("npc_neutral_placeholder", Position.world{ 460, 130 }, { "npc", "shimmy_customer" })
        :trait_scriptable{ script_id = "Sally", story_id = "Sally" }
        :trait_movement{ speed = 2.0 }
        :trait_speech{ file = "This curfew nonsense is a bad fucking joke..." }

    scene:entity("npc_neutral_placeholder", Position.world{ 400, 130 }, { "npc", "shimmy_customer" })
        :trait_scriptable{ script_id = "Bill", story_id = "Bill" }
        :trait_movement{ speed = 2.0 }
        :trait_speech{ file = "Shimmy really knows how to brew a strong drink." }

    scene:entity("table_simple_wooden_long", Position.iso{ 490, 408 }, { "furniture", "wood" })
        :trait_customsort({ 0, 265 }, { 378, 110 })

    --     {
    --         "prefab": "table_simple_wooden_round",
    --         "position": { "iso": [ 670, 679 ] }
    --     },
    --     {
    --         "prefab": "table_simple_wooden_round",
    --         "position": { "iso": [ 890, 579 ] }
    --     },
    --     {
    --         "prefab": "table_simple_wooden_round",
    --         "position": { "iso": [ 842, 800 ] }
    --     },
    --     {
    --         "prefab": "table_simple_wooden_round",
    --         "position": { "iso": [ 1047, 720 ] }
    --     },
    --     {
    --         "prefab": "door_simple_wooden_single",
    --         "position": { "iso": [ 993, 203 ] },
    --         "tags": [ "locked", "-y" ]
    --     }
    -- ]
end

