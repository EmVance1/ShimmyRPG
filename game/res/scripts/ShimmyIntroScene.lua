

function OnStartAsync()
    local shimmy = shmy.entity("ShimmyPrime")

    shmy.camera.set_target(shimmy:get_position());

    shmy.set_mode(shmy.mode.CINEMATIC)
    shmy.yield_seconds(1.0)
    shmy.yield_to_dialogue("res/scripts/ShimmyIntroScene.shmy")


    shmy.flags["ShimmyIntroScene_HasPlayed"] = 1

    -- shimmy:set_path({ x=400, y=50 })
    shmy.set_mode(shmy.mode.COMBAT)
    shmy.yield_to_combat({}, { "shimmy_staff", "shimmy_customer" })

    shmy.yield_seconds(0.7)
    shmy.set_mode(shmy.mode.NORMAL)
end

