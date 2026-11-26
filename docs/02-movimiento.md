# Ruedas, encoders y odometría

[Índice](README.md)

## Activación

```xml
<!-- Dentro de actuators -->
<differential_steering implementation="default"/>
<!-- Dentro de sensors -->
<miniqupa_encoder implementation="default"/>
```

## Órdenes de ruedas

```lua
robot.wheels.set_velocity(3, 3)    -- Avanza a 3 cm/s.
robot.wheels.set_velocity(-3, -3)  -- Retrocede.
robot.wheels.set_velocity(-2, 2)  -- Gira a la izquierda sobre el centro.
robot.wheels.set_velocity(2, -2)  -- Gira a la derecha.
robot.wheels.set_velocity(0, 0)   -- Parada.
```

Argumentos: izquierda y derecha, en **cm/s**, con signo. Son velocidades lineales de las ruedas, no PWM ni rad/s. El radio de rueda es 0,016 m y el eje efectivo mide 0,0795 m. El motor físico de ARGoS trabaja internamente en m/s; no conviertas dos veces al usar esta API.

Si tienes una velocidad de cuerpo `v` en m/s y velocidad angular `w` en rad/s, calcula:

```lua
local eje = 0.0795
local izquierda = 100 * (v - w * eje / 2)
local derecha   = 100 * (v + w * eje / 2)
robot.wheels.set_velocity(izquierda, derecha)
```

Una colisión puede impedir que el movimiento real coincida con la orden. Las velocidades máximas y la dinámica física aún no están calibradas con el hardware.

## Lectura del encoder

Dentro de `step()`:

```lua
local e = robot.wheels.encoder
log("izq=" .. e.distance_left .. " der=" .. e.distance_right)
```

| Campo Lua | C++ en `GetReading()` | Unidad |
|---|---|---|
| `distance_left` | `CoveredDistanceLeftWheel` | Metros con signo desde la muestra anterior |
| `distance_right` | `CoveredDistanceRightWheel` | Metros con signo desde la muestra anterior |
| `axis_length` | `WheelAxisLength` | Separación del eje en metros |

Son **incrementos**, no un contador acumulado. Sumarlos una sola vez por paso. No multiplicarlos por `dt`: ya son distancias. Para estimar velocidad media de una rueda, divide su incremento por el tiempo entre muestras.

```lua
-- Estado creado fuera de las funciones del controlador:
local recorrido = 0
-- Dentro de step():
local e = robot.wheels.encoder
recorrido = recorrido + (e.distance_left + e.distance_right) / 2
```

`recorrido` es avance firmado; retroceder lo reduce. Para longitud de camino usa el valor absoluto del incremento central, teniendo en cuenta que un giro puro no añade traslación central.

## Odometría planar aproximada

```lua
-- Estado persistente, a poner a cero en init() y reset():
local x, y, theta = 0, 0, 0
-- Dentro de step():
local e = robot.wheels.encoder
local ds = (e.distance_left + e.distance_right) / 2
local dtheta = (e.distance_right - e.distance_left) / e.axis_length
x = x + ds * math.cos(theta + dtheta / 2)
y = y + ds * math.sin(theta + dtheta / 2)
theta = theta + dtheta
```

Usa un marco relativo a tu pose inicial; no lee coordenadas globales del simulador. Es integración de punto medio, aproximada para pasos de giro grandes. El [ejemplo de obstáculos](../examples/avoidance.lua) acumula distancia para verificar que hay movimiento.

## Equivalente C++

```cpp
#include <argos3/plugins/robots/generic/control_interface/ci_differential_steering_actuator.h>
#include <argos3/plugins/robots/miniqupa/control_interface/ci_miniqupa_encoder_sensor.h>
// En Init():
auto* ruedas = GetActuator<argos::CCI_DifferentialSteeringActuator>("differential_steering");
auto* encoder = GetSensor<argos::CCI_MiniQupaEncoderSensor>("miniqupa_encoder");
// En ControlStep():
ruedas->SetLinearVelocity(3, 3); // cm/s
const auto& e = encoder->GetReading();
```

## Qué simula

El encoder se deriva del movimiento real del cuerpo durante los subpasos físicos, bajo rodadura ideal. No cuenta pulsos ni representa cada eje de motor de forma independiente. Si el cuerpo está bloqueado no debe interpretarse como un motor girando con patinamiento. Tras reset los incrementos se reinician; restablece también tu odometría. No es un sensor de velocidad de ruedas adicional ni una IMU.
