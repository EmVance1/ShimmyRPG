

local shimmy = nil

function OnStartAsync()
    ---@class Entity
    shimmy = shmy.entity("ShimmyPrime")

    shmy.camera.set_target(shimmy:get_position())

    shmy.set_mode(shmy.mode.CINEMATIC)
    shmy.yield_to_dialogue("speech/ShimmyIntroScene.shmy")

    shmy.flags["ShimmyIntroScene_HasPlayed"] = 1

    shmy.events.send("ShimmyIntroScene_HasPlayed", { friendly=0 })

    shimmy:set_path({ x=400, y=50 }):on_arrival("ShimmyReachBar");
end


shmy.event = {
    ON_ARRIVAL_ID = 0x0,
    MESSAGE_ID    = 0x1,
    AND = 0x1000,
    OR  = 0x1001,

    _CreateEvent = function(table)
        local meta = { __band=shmy.event.AndConjunction, __bor=shmy.event.OrConjunction }
        setmetatable(table, meta)
        return table
    end,

    AndConjiunction = function(A, B) return shmy.event._CreateEvent{ id=shmy.event.AND, lhs=A, rhs=B } end,
    OrConjunction   = function(A, B) return shmy.event._CreateEvent{ id=shmy.event.OR, lhs=A, rhs=B } end,

    OnArrival = function(entity, position) return shmy.event._CreateEvent{ id=shmy.event.ON_ARRIVAL_ID } end,
    Message   = function(msg) return shmy.event._CreateEvent{ id=shmy.event.MESSAGE_ID } end
}


shmy.event.listen{
    event = shmy.event.OnArrival() & shmy.event.Message(),
    filter = function(entity, _, _) return entity == "ShimmyPrime" end,
    action = function(_,      _, _)
        shmy.gui.popup.new("Shimmy is waiting for you.", shmy.gui.popup.CloseButton)
    end
}


function where(x) end


--[listener]
if shmy.event.OnArrival(entity, position) & shmy.event.Message(msg) - where (entity == "ShimmyPrime") then
    shmy.gui.popup.new("Shimmy is waiting for you.", shmy.gui.popup.CloseButton)
end

-- #[listener]
shmy.event.listen{ event = shmy.event.OnArrival() & shmy.event.Message(), filter = function(entity, _, _) return entity == "ShimmyPrime" end, action = function(_,      _, _)
    shmy.gui.popup.new("Shimmy is waiting for you.", shmy.gui.popup.CloseButton)
end
}

