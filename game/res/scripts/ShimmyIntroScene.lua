---@diagnostic disable: lowercase-global
---@type fun(mode: integer)
set_mode = set_mode

---@diagnostic disable: lowercase-global
---@type fun(pos: table)
set_camera = set_camera

---@diagnostic disable: lowercase-global
---@type fun(entity: string): table
get_position = get_position

---@diagnostic disable: lowercase-global
---@type fun(file: string): nil
start_dialogue = start_dialogue

---@diagnostic disable: lowercase-global
---@type fun(entity: string, pos: table)
set_path = set_path

---@diagnostic disable: lowercase-global
---@type fun(name: string, val: integer)
set_flag = set_flag

---@diagnostic disable: lowercase-global
---@type fun(name: string): integer
get_flag = get_flag


Modes = {
    NORMAL = 0,
    CINEMATIC = 1,
    COMBAT = 2,
}


function OnStartAsync()
    set_mode(Modes.CINEMATIC)

    set_camera(get_position("ShimmyPrime"));

    coroutine.yield(1.0)

    start_dialogue("res/scripts/ShimmyIntroScene.shmy")

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

