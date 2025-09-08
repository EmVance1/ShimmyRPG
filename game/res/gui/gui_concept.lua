

function Root(ctx, scene)
    PanelBegin{ id="combat", bottomcenter="0 50", size=function() return { ctx.win.x - 800, 150 } end, hidden=true }
        TextBegin{ id="current_actor", topleft="0 -50", size="200 50" }TextEnd{}
        Button{ id="end_turn", topright="0 -50", size="150 150", texture="0 0 150 150", onclick=function() scene.combat.advance_turn = true end }
    PanelEnd{}

    -- combat_gui.select("#end_turn")
    --     ->set_callback([&](){ combat_mode.advance_turn = true; });
    -- gui.add_widget("combat", combat_gui);
end

