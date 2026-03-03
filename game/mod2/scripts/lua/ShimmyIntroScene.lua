local shmy = shmy


RegisterAsyncHandler("OnShimmyIntroCutsceneTriggered", function(state, _)
    shmy.set_mode(shmy.mode.CINEMATIC, true)
    state.shimmy = shmy.entity("Shimmy")
    shmy.camera.set_target(state.shimmy:get_position())
    shmy.yield_seconds(1.0)

    shmy.yield_to_dialogue("speech/ShimmyIntroScene.shmy")
end)

RegisterAsyncHandler("OnShimmyIntroCutsceneFinished", function(state, _)
    shmy.flags["ShimmyIntroScene.HasPlayed"] = true
    state.shimmy:set_path({ x=2.4, y=-0.8 })
    shmy.yield_seconds(0.4)
    shmy.set_mode(shmy.mode.NORMAL)
end)



RegisterHandler("OnEntityDestinationReached", function(_, _)
    print("reached dest")
end)

RegisterHandler("PrintDebug", function(_, args)
    print(args[1])
end)

