
-- local function protect(tbl)
--     return setmetatable({}, {
--         __index = tbl,
--         __newindex = function(_, key, value)
--             error("attempting to change constant " ..
--                    tostring(key) .. " to " .. tostring(value), 2)
--         end
--     })
-- end
-- 
Modes = {
    NORMAL = 0,
    CINEMATIC = 1,
    COMBAT = 2,
}
-- Modes = protect(Modes)


function OnStartAsync()
    set_mode(Modes.CINEMATIC)

    set_camera(get_position("ShimmyPrime"));

    coroutine.yield(1.0)

    start_dialogue("res/scripts/ShimmyIntroScene.dia")

    coroutine.yield(0)

    set_flag("ShimmyIntroScene_HasPlayed", 1)

    if get_flag("ShimmyHostility") > 2 then
        -- set_mode(Modes.COMBAT, { "shimmy_staff", "shimmy_customer" }, {})
    else
        set_path("ShimmyPrime", { x=400, y=50 })

    end

    coroutine.yield(0.7)

    set_mode(Modes.NORMAL)
end

