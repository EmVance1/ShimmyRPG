
Position = {
    WORLD = 0,
    ISO = 1,

    world = function(pos) return { x=pos[1], y=pos[2], mode=Position.WORLD } end
    iso =   function(pos) return { x=pos[1], y=pos[2], mode=Position.ISO } end
}


function Setup(scene)
    scene.label = "Shimmy's Shack"
    scene.topleft = { 948, 315 }
    scene.scale = 5.35

    scene.background = {
        { "ademmar/shimmyshack.png", { -68, -70, 2054, 1156 } }
    },

    -- front door
    scene:trigger_rect({ 265, 535, 120, 30 }, Action.goto{ index = 1, spawnpos = { 810, 360 }, suppress_triggers = true })
        :condition("!Ademmar_CurfewInPlace")

    -- welcome cutscene
    scene:trigger_rect({ 140, 0, 394, 380 },  Action.script("res/scripts/ShimmyIntroScene.lua"))
        :condition("ShimmyIntroScene_IsActive & once")

    -- curfew blocker
    scene:trigger_rect({ 265, 535, 120, 30 }, Action.popup("There is a curfew in place. It is unwise to go outside."))
        :condition("Ademmar_CurfewInPlace")

    -- behind bar
    scene:trigger_rect({ 192, 0, 342, 100 },  Action.dialogue("res/scripts/ShimmyBehindBar.shmy"))


    scene:entity("player_placeholder", Position.world{ 320, 480 }, { "player" })
        :can_script{ script = "player", story = "Stanza" }
        :can_move{ speed = 2.5 }

    scene:entity("anim_test", Position.world{ 330, 50 }, { "npc", "trader", "shimmy_staff" })
        :can_script{ script = "ShimmyPrime", story = "Shimmy" }
        :can_move{ speed = 2.0 }
        :can_speak{ file = "res/scripts/ShimmyDialogue.shmy" }

    scene:entity("npc_friendly_placeholder", Position.world{ 500, 290 }, { "npc", "shimmy_customer" })
        :can_script{ script = "BrianPrime", story = "Brian" }
        :can_move{ speed = 2.0 }
        :can_speak{ file = "res/scripts/BrianDialogue.shmy" }

    scene:entity("npc_neutral_placeholder", Position.world{ 460, 130 }, { "npc", "shimmy_customer" })
        :can_script{ script = "Sally", story = "Sally" }
        :can_move{ speed = 2.0 }
        :can_speak{ file = "This curfew nonsense is a bad fucking joke..." }

    scene:entity("npc_neutral_placeholder", Position.world{ 400, 130 }, { "npc", "shimmy_customer" })
        :can_script{ script = "Bill", story = "Bill" }
        :can_move{ speed = 2.0 }
        :can_speak{ file = "Shimmy really knows how to brew a strong drink." }

    scene:entity("table_simple_wooden_long", Position.iso{ 490, 408 }, { "furniture", "wood" })
        :boundary({ 0, 265 }, { 378, 110 })

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

