local shmy = shmy


RegisterAsyncHandler("OnShimmyIntroCutsceneTriggered", function(args)
    local shimmy = shmy.entity("ShimmyPrime")

    shmy.camera.set_target(shimmy:get_position())
    shmy.set_mode(shmy.mode.CINEMATIC)
    shmy.yield_seconds(1.0)
    shmy.yield_to_dialogue("speech/ShimmyIntroScene.shmy")

    shmy.flags["ShimmyIntroScene.HasPlayed"] = 1

    shimmy:set_path({ x=400, y=50 })
    shmy.set_mode(shmy.mode.NORMAL)
end)

