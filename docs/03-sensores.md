# Sensores IR y color inferior

[Índice](README.md)

## Los ocho IR

```xml
<miniqupa_ir implementation="default" min_range="0.04" noise_level="0" show_rays="false"/>
```

Este bloque va en `<sensors>`. En la entidad `<miniqupa ir_range="0.40">` se establece el máximo. `min_range` debe ser positivo y menor que el máximo; `noise_level` finito y no negativo. Los números del XML usan punto decimal.

```lua
for i, lectura in ipairs(robot.miniqupa_ir) do
   log(lectura.id .. ": " .. lectura.value)
end
```

| Campo Lua | C++ `SReading` | Significado |
|---|---|---|
| `id` | `Id` | Etiqueta CAD `IR1`…`IR8` |
| `value` | `Value` | Proximidad normalizada [0,1]: mayor significa más cerca |
| `angle` | `Angle` | Yaw local en radianes, positivo a la izquierda |
| `detected` | `Detected` | Diagnóstico ideal de intersección dentro del alcance |
| `distance_m` | `Distance` | Diagnóstico ideal de distancia desde las lentes, m |
| `range_m` | `Range` | Alcance máximo, m |

En Lua los índices son 1…8; en el vector C++ son 0…7. Es preferible identificar canales por `id` al definir comportamientos. Las etiquetas CAD todavía no garantizan una correspondencia con los índices del firmware físico.

| ID | Orientación local aproximada |
|---|---:|
| IR1 | +150° |
| IR2 | +15°, frente izquierdo |
| IR3 | −15°, frente derecho |
| IR4 | +50° |
| IR5 | +90° |
| IR6 | −50° |
| IR7 | −90° |
| IR8 | −150° |

Las posiciones respetan las asimetrías CAD: no son ocho rayos separados uniformemente 45°. La geometría exacta está en [config/miniqupa_geometry.json](../config/miniqupa_geometry.json).

### Interpretación

- Sin intersección dentro del alcance: `value=0`, `detected=false`, `distance_m=range_m`, con ruido apagado.
- Hasta 4 cm: saturación `value=1`, no lectura de distancia realista a esa resolución.
- Entre 4 y 40 cm: curva tipo foot-bot reescalada, no relación lineal ni calibración del Sharp.
- `noise_level` suma ruido uniforme y recorta a [0,1]; puede producir un valor positivo aun sin detección ideal.

Usa `value` para diseñar un control basado en proximidad. `distance_m` y `detected` son diagnósticos ideales: no añadas dependencias de ellos si buscas reproducir solo la salida real normalizada.

```lua
local cerca = false
for _, ir in ipairs(robot.miniqupa_ir) do
   if (ir.id == "IR2" or ir.id == "IR3") and ir.value > 0.15 then
      cerca = true
   end
end
if cerca then robot.wheels.set_velocity(-2, 2)
else robot.wheels.set_velocity(3, 3) end
```

El umbral 0,15 es una decisión de este ejemplo, no una distancia calibrada. El [ejemplo completo](../examples/avoidance.argos) coloca una caja delante y verifica que el robot cambia de orden al detectarla.

### C++

```cpp
#include <argos3/plugins/robots/miniqupa/control_interface/ci_miniqupa_ir_sensor.h>
// Init():
auto* ir = GetSensor<argos::CCI_MiniQupaIRSensor>("miniqupa_ir");
// ControlStep():
for(const auto& lectura : ir->GetReadings()) {
   if(lectura.Id == "IR2" && lectura.Value > 0.15) { /* obstáculo */ }
}
```

El modelo es un rayo por módulo, sin cono, reflectancia ni textura superficial. Ignora el cuerpo propio y ve las colisiones simplificadas de otros objetos. `show_rays` sirve de diagnóstico en la visualización.

## Sensor RGB inferior

```xml
<miniqupa_floor_color implementation="default" show_rays="false"/>
```

```lua
local suelo = robot.floor_color
if suelo.valid then
   local c = suelo.color
   if c.red > 200 and c.green < 50 and c.blue < 50 then
      robot.wheels.set_velocity(0, 0)
   end
end
```

`color.red`, `.green`, `.blue` son canales 0…255. `valid` es booleano: negro es un color válido, no un error. Tras reset es falso hasta una actualización. El sensor consulta un punto del suelo bajo su posición local; no detecta el color de robots o cajas.

Su posición se genera desde el CAD. Puedes reemplazarla con `offset="x,y,z"` en metros en el bloque del sensor; Z debe ser no negativo. No cambies ese valor para fabricar artificialmente un color: configura el suelo del escenario.

El ejemplo usa `<floor source="loop_functions" .../>` sin personalización, que resulta blanco. Para zonas coloreadas configura una imagen de suelo de ARGoS o implementa `GetFloorColor()` en tus loop functions. Si usas una imagen, la resolución/mapeo del suelo también influye en el punto muestreado.

```cpp
#include <argos3/plugins/robots/miniqupa/control_interface/ci_miniqupa_floor_color_sensor.h>
auto* suelo = GetSensor<argos::CCI_MiniQupaFloorColorSensor>("miniqupa_floor_color");
const auto& lectura = suelo->GetReading();
if(lectura.Valid) {
   auto rojo = lectura.Color.GetRed();
}
```

Es RGB ideal, no datos crudos RGBC del TCS34725. No simula iluminación, ganancia, tiempo de integración, espectro, superficie de muestreo ni oclusión del haz inferior. No mide feromonas ni modifica el suelo.
