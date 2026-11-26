-- Control de proximidad: umbral del ejemplo, no distancia calibrada.
local tick, distancia, giros
function init()
   tick, distancia, giros = 0, 0, 0
   robot.diffuser.set_color(0, 255, 0)
end
function step()
   tick = tick + 1
   local cerca = false
   for _, ir in ipairs(robot.miniqupa_ir) do
      assert(ir.value >= 0 and ir.value <= 1)
      if (ir.id == "IR2" or ir.id == "IR3") and ir.value > 0.15 then
         cerca = true
      end
   end
   local e = robot.wheels.encoder
   distancia = distancia + math.abs((e.distance_left + e.distance_right) / 2)
   if cerca then
      giros = giros + 1
      robot.wheels.set_velocity(-2, 2)
      robot.diffuser.set_color(255, 0, 0)
   else
      robot.wheels.set_velocity(3, 3)
      robot.diffuser.set_color(0, 255, 0)
   end
   if tick == 100 then
      robot.wheels.set_velocity(0, 0)
      assert(giros > 0, "La caja no produjo la respuesta de giro")
      assert(distancia > 0.01, "No se observo avance en el encoder")
      log("MINIQUPA_AVOIDANCE_OK")
   end
end
function reset() init() end
function destroy() end
