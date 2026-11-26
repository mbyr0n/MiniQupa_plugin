-- Los IDs lógicos del protocolo se asignan aquí; no vienen de la radio.
local tick, mi_id, otro_id, visto_radio, visto_rab, visto_blob
function init()
   tick = 0
   visto_radio, visto_rab, visto_blob = false, false, false
   if robot.id == "mq0" then
      mi_id, otro_id = 1, 2
      robot.diffuser.set_color(255, 0, 0)
   else
      mi_id, otro_id = 2, 1
      robot.diffuser.set_color(0, 0, 255)
   end
   robot.range_and_bearing.clear_data()
end
function recibir_radio()
   for _, m in ipairs(robot.simple_radios.espnow.recv) do
      assert(m[1] == otro_id and m[2] == 7 and m[3] == 125)
      visto_radio = true
   end
end
function recibir_rab()
   for _, p in ipairs(robot.range_and_bearing) do
      -- El contenido inicial puede ser cero antes de propagarse la primera orden.
      if p.data[1] == otro_id then
         assert(#p.data == 10 and p.data[2] == 9 and p.data[10] == 255)
         assert(math.abs(p.range - 30) < 0.01, "RAB debe dar centimetros")
         assert(type(p.horizontal_bearing) == "number")
         assert(type(p.vertical_bearing) == "number")
         visto_rab = true
      end
   end
end
function observar()
   for _, b in ipairs(robot.camera.blobs) do
      if mi_id == 1 then assert(b.color.blue == 255 and b.color.red == 0)
      else assert(b.color.red == 255 and b.color.blue == 0) end
      assert(b.angular_width > 0 and b.samples > 0)
      visto_blob = true
   end
   if robot.floor_color.valid then
      local c = robot.floor_color.color
      assert(c.red == 255 and c.green == 255 and c.blue == 255)
   end
end
function step()
   tick = tick + 1
   robot.wheels.set_velocity(0, 0)
   recibir_radio()
   recibir_rab()
   observar()
   robot.simple_radios.espnow.send({mi_id, 7, 125})
   robot.range_and_bearing.set_data({mi_id, 0, 0, 0, 0, 0, 0, 0, 0, 255})
   robot.range_and_bearing.set_data(2, 9)
   if tick == 100 then
      assert(visto_radio and visto_rab and visto_blob, "Falta recepcion de un dispositivo")
      assert(robot.camera.raw_samples > 0)
      log("MINIQUPA_NEIGHBORS_OK " .. robot.id)
   end
end
function reset() init() end
function destroy() end
