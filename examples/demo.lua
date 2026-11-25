-- Velocidades: cm/s. Lecturas del encoder: metros por paso de control.
local tick
function init()
   tick = 0
   robot.diffuser.set_color(0, 0, 255)
end
function step()
   tick = tick + 1
   -- Este ejemplo avanza dos segundos y después se detiene.
   if tick <= 40 then robot.wheels.set_velocity(2, 2)
   else robot.wheels.set_velocity(0, 0) end
   robot.simple_radios.espnow.send({1, 2, 3})
   robot.range_and_bearing.set_data({7, 0, 0, 0, 0, 0, 0, 0, 0, 255})
   if tick == 100 then log("MINIQUPA_DEMO_DONE") end
end
function reset() init() end
function destroy() end
