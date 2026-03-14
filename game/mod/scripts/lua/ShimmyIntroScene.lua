local shmy = shmy


RegisterHandler("OnStart", function(state, _)
    state.shimmy = shmy.entity("Shimmy")
    state.brian  = shmy.entity("BrianPrime")
end)

RegisterAsyncHandler("OnShimmyIntroCutsceneTriggered", function(state, _)
    shmy.set_mode(shmy.mode.CINEMATIC, true)
    shmy.camera.set_target(state.shimmy:get_position())
    shmy.yield_seconds(1.0)

    shmy.yield_to_dialogue("ShimmyDialogue.IntroScene")
    -- shmy.yield_to_dialogue("experiments/fib.Default")
end)

RegisterAsyncHandler("OnShimmyIntroCutsceneFinished", function(state, _)
    shmy.flags["ShimmyIntroScene.HasPlayed"] = true
    state.shimmy:set_path({ x=2.4, y=-0.8 })
    shmy.yield_seconds(0.4)
    shmy.set_mode(shmy.mode.NORMAL)

    state.brian.queue:add_action(shmy.action.UsePortal{ "front_door" })
    -- state.brian.queue:await(shmy.condition.Flag{ key="Player.Outside", val=1 })
    -- state.brian.queue:add_action(shmy.action.SetPath{ { x=-3.8, y=-2.3 } })
end)


RegisterHandler("PrintDebug", function(_, args)
    print(args[1])
end)

