

function CombatHud(ctx, scene)
    return Panel({ id="combat", bottomcenter="0 50", size=function() return { ctx.viewport.x - 800, 150 } end, hidden=true },
        Text({ id="current_actor", topleft="0 -50", size="200 50" }),
        Button({ id="end_turn", topright="0 -50", size="150 150", texture="0 0 150 150", onclick=scene.combat.advance_turn })
    )
end

