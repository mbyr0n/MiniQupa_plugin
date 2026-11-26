# Primer controlador

[Índice](README.md) · [Siguiente: movimiento](02-movimiento.md)

## Preparar la terminal

Desde la raíz del repositorio, después de compilar e instalar como indica el README:

```bash
export MINIQUPA_PREFIX="$PWD/install"
export ARGOS_PLUGIN_PATH="$MINIQUPA_PREFIX/lib/argos3${ARGOS_PLUGIN_PATH:+:$ARGOS_PLUGIN_PATH}"
export LD_LIBRARY_PATH="$MINIQUPA_PREFIX/lib/argos3${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}"
argos3 -c examples/demo.argos -z
```

Quita `-z` para abrir la visualización Qt/OpenGL. Las rutas de los scripts Lua son relativas al **directorio de trabajo**, por eso debes ejecutar desde la raíz del repositorio. Usa bibliotecas pertenecientes a la misma instalación ARGoS.

## Ciclo de vida Lua

```lua
function init()
   -- Una vez al crear el controlador. Inicializa tu estado y colores.
   robot.diffuser.set_color(0, 0, 255)
end
function step()
   -- Se repite: lee la muestra actual y establece la siguiente orden.
   robot.wheels.set_velocity(2, 2)
end
function reset()
   -- Restablece también tus contadores y odometría al reiniciar.
   init()
end
function destroy()
   -- Libera archivos u otros recursos que hayas abierto.
end
```

`init()` no sustituye la primera actualización de sensores; no dependas de una medida válida dentro de él. En `step()`, los sensores reflejan el estado disponible antes de aplicar la orden de ese paso. El movimiento aparecerá en lecturas posteriores. Una orden de ruedas persiste hasta que la cambies; utiliza explícitamente `set_velocity(0,0)` para parar. No dependas de `destroy()` como orden de frenado durante una simulación.

`robot.id` identifica al robot local. No proporciona la identidad de los emisores de blobs o de paquetes recibidos: para comunicación, añade un identificador en tus datos si lo necesitas.

## Relación entre XML y programa

```xml
<controllers>
  <lua_controller id="mi_control" library="libargos3plugin_simulator_miniqupa">
    <actuators>
      <differential_steering implementation="default"/>
      <miniqupa_diffuser implementation="default" medium="leds"/>
    </actuators>
    <sensors>
      <miniqupa_ir implementation="default"/>
      <miniqupa_encoder implementation="default"/>
    </sensors>
    <params script="examples/avoidance.lua"/>
  </lua_controller>
</controllers>
```

El atributo `id="mi_control"` se referencia en la entidad mediante `<controller config="mi_control"/>`. El dispositivo solo está disponible en el programa si aparece en su controlador. No basta con que la pieza exista en la malla visual.

Los fragmentos de otros capítulos se añaden **dentro de** `<actuators>` o `<sensors>`, sin crear bloques duplicados. Si un actuador exige un medio, decláralo también en `<media>`. Ejemplo:

```xml
<media>
  <led id="leds" grid_size="10,10,5"/>
</media>
```

Para un experimento completo utiliza los XML de `examples/`. El [README](../README.md#configurar-el-robot) enumera los atributos de entidad. Se incluyen allí límites de IR, referencias virtuales de radio/RAB, geometría del tag y parámetros físicos provisionales.

## Frecuencia y tiempo

```xml
<framework>
  <system threads="0"/>
  <experiment length="5" ticks_per_second="20" random_seed="1"/>
</framework>
```

Aquí 40 pasos equivalen a 2 s. Si cambias `ticks_per_second`, cambia también los cálculos de tiempo de tu controlador. Ni las velocidades ni las distancias del encoder deben multiplicarse por el número de iteraciones físicas internas.

## Depurar

Usa `log("mensaje")` en Lua y observa la consola. En C++ usa `argos::LOG`. Evita imprimir todas las medidas en cada paso cuando simules muchos robots. Si aparece un campo Lua inexistente, revisa primero el nombre XML, la implementación y el medio; no añadas tablas Lua vacías para ocultar la falta de dispositivo.
