# Difusor RGB y cámara panorámica de blobs

[Índice](README.md)

## Activar los dispositivos

```xml
<!-- actuators -->
<miniqupa_diffuser implementation="default" medium="leds"/>
<!-- sensors -->
<miniqupa_camera implementation="default" medium="leds"/>
<!-- media -->
<led id="leds" grid_size="10,10,5"/>
```

Los tres fragmentos van en sus secciones respectivas. El emisor y observador deben usar el mismo medio.

## Cambiar el difusor

```lua
robot.diffuser.set_color(255, 0, 0)   -- Rojo.
robot.diffuser.set_color(0, 255, 0)   -- Verde.
robot.diffuser.set_color(0, 0, 255)   -- Azul.
robot.diffuser.set_color(255, 255, 0) -- Amarillo.
robot.diffuser.set_color(0, 0, 0)     -- Apagado.
```

Requiere tres enteros entre 0 y 255. El color persiste hasta otra orden; reset lo apaga. Hay un único comando común para el difusor iluminado por los cinco LEDs físicos, sin API de control individual de cada LED. Negro apaga la emisión, no pinta de negro el material visual apagado.

```cpp
#include <argos3/plugins/robots/miniqupa/control_interface/ci_miniqupa_diffuser_actuator.h>
auto* difusor = GetActuator<argos::CCI_MiniQupaDiffuserActuator>("miniqupa_diffuser");
difusor->SetColor(argos::CColor(255, 0, 0));
difusor->TurnOff();
```

## Leer los blobs

```lua
for _, blob in ipairs(robot.camera.blobs) do
   local c = blob.color
   log("RGB=" .. c.red .. "," .. c.green .. "," .. c.blue ..
       " angulo=" .. blob.angle .. " ancho=" .. blob.angular_width)
end
```

| Lua | C++ `SBlob` | Interpretación |
|---|---|---|
| `color` | `Color` | RGB del emisor ideal |
| `angle` | `Angle` | Azimut local en radianes; 0 delante, positivo a la izquierda |
| `angular_width` | `AngularWidth` | Ancho angular del segmento, radianes |
| `samples` | `Samples` | Número diagnóstico de muestras ópticas agrupadas |

`robot.camera.raw_samples` / `GetRawSampleCount()` es el número total de muestras visibles antes de agrupar. **No son píxeles ni número de robots**. Los campos se actualizan por muestra; no acumules el vector como un historial sin hacerlo tú mismo.

Un blob no contiene `id`, pose global ni distancia. Dos emisores próximos con el mismo color pueden fusionarse. Un difusor puede aparecer dividido si la visibilidad rompe la continuidad. La agrupación usa color y separación angular, incluyendo el cruce −π/π; no usa la identidad del robot origen. El cuerpo propio se excluye, por lo que la cámara no ve su propio difusor.

### Parámetros ópticos ideales

```xml
<miniqupa_camera implementation="default" medium="leds"
  range="0.8" height="0.15" vertical_half_angle_deg="45"
  merge_gap_deg="1" show_rays="false"/>
```

| Parámetro | Unidad y efecto |
|---|---|
| `range` | Metros de alcance horizontal; positivo; inicial 0,8 |
| `height` | Metros de altura virtual de observación; positivo; inicial 0,15 |
| `vertical_half_angle_deg` | Semiapertura vertical en grados, (0,90]; inicial 45 |
| `merge_gap_deg` | Holgura de unión angular en grados, [0,180]; inicial 1 |
| `sample_radius` | Radio virtual de cada parche luminoso en metros, ≥0; inicial π·0,0544/16 ≈0,01068 |
| `show_rays` | Diagnóstico de líneas de visión |

No son parámetros calibrados de la OV5647 o del espejo. Aunque la ventana muestre el espejo, el sensor no calcula reflexión por trazado óptico ni entrega una imagen. Emisores negros y muestras ocluidas por cuerpos no se observan. La envolvente simplificada de colisión participa en esas oclusiones.

### C++

```cpp
#include <argos3/plugins/robots/miniqupa/control_interface/ci_miniqupa_camera_sensor.h>
auto* camara = GetSensor<argos::CCI_MiniQupaCameraSensor>("miniqupa_camera");
for(const auto& blob : camara->GetBlobs()) {
   const auto angulo = blob.Angle.GetValue(); // radianes
}
```

## Ejemplo práctico

[neighbors.argos](../examples/neighbors.argos) coloca dos robots estacionarios separados 30 cm. Uno enciende rojo y otro azul. Cada uno comprueba que ve un blob del otro color y que no ve su propia emisión. También intercambian datos por radio y RAB. Este escenario relaciona las tres percepciones sin asumir que un color identifique siempre a un único robot.
