local shmy = shmy


RegisterHandler("OnStart", function(state, _)
    state.shimmy = shmy.entity("ShimmyPrime")
end)

RegisterAsyncHandler("OnShimmyIntroCutsceneTriggered", function(state, _)
    shmy.camera.set_target(state.shimmy:get_position())
    shmy.set_mode(shmy.mode.CINEMATIC)
    shmy.yield_seconds(1.0)
    shmy.yield_to_dialogue("speech/ShimmyIntroScene.shmy")

    shmy.flags["ShimmyIntroScene.HasPlayed"] = 1

    state.shimmy:set_path({ x=400, y=50 })
    shmy.set_mode(shmy.mode.NORMAL)
end)



RegisterHandler("OnEntityDestinationReached", function(_, _)
    print("reached dest")
end)

