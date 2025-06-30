

function OnStartAsync()
    local shimmy = shmy.entity("ShimmyPrime")

    shmy.set_mode(shmy.mode.CINEMATIC)

    shmy.camera.set_target(shimmy:get_position());

    shmy.wait_seconds(1.0)

    shmy.yield_to_dialogue("res/scripts/ShimmyIntroScene.shmy")

    shimmy:set_path({ x=400, y=50 })

    shmy.flags["ShimmyIntroScene_HasPlayed"] = 1

    -- shmy.set_combat({}, { "shimmy_staff", "shimmy_customer" })
    -- shmy.set_mode(shmy.mode.COMBAT)

    shmy.wait_seconds(0.7)

    shmy.set_mode(shmy.mode.NORMAL)
end

