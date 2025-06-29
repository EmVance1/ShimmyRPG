---@diagnostic disable: lowercase-global
---@type fun(mode: integer)
set_mode = set_mode

---@diagnostic disable: lowercase-global
---@type fun(pos: table)
camera_set_pos = camera_set_pos

---@diagnostic disable: lowercase-global
---@type fun(pos: table)
camera_zoom = camera_zoom

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
    DIALOGUE = 1,
    CINEMATIC = 2,
    COMBAT = 3,
}


function OnStartAsync()
    set_mode(Modes.CINEMATIC)

    camera_set_pos(get_position("ShimmyPrime"));

    coroutine.yield(1.0)

    start_dialogue("res/scripts/ShimmyIntroScene.shmy")

    coroutine.yield(0)

    set_flag("ShimmyIntroScene_HasPlayed", 1)

    set_combat({}, { "shimmy_staff", "shimmy_customer" })
    set_mode(Modes.COMBAT)
    -- set_path("ShimmyPrime", { x=400, y=50 })

    coroutine.yield(0.7)

    set_mode(Modes.NORMAL)
end

