# Manual de programación de MiniQupa

Este manual explica **las funciones que implementa este plugin**, con Lua para comenzar y equivalentes C++ para controladores compilados. La instalación está en el [README principal](../README.md#compilar-e-instalar). No hace falta ROS ni el workspace de desarrollo.

## Ruta de aprendizaje

1. [Primer controlador y configuración](01-primer-controlador.md): ciclo de ejecución, XML, unidades y cómo ejecutar.
2. [Ruedas, encoders y odometría](02-movimiento.md): avanzar, girar, detenerse y acumular desplazamiento.
3. [IR y color inferior](03-sensores.md): los ocho canales, validez y lectura del suelo.
4. [Difusor y cámara de blobs](04-vision.md): colores, agrupación, dirección y límites ópticos.
5. [Simple radios y range-and-bearing](05-comunicacion.md): mensajes, unidades, índices y diferencias entre APIs.
6. [Tags y configuración del escenario](06-tags.md): identificadores, orientación y detección externa.
7. [Controladores C++](07-cpp.md): ejemplo compilable que consulta todos los dispositivos de control.
8. [Ejemplos y verificación](08-ejemplos.md): resultados esperados y cómo repetir las comprobaciones.

Cada capítulo incluye el XML necesario, ejemplos, campos de lectura y límites. Los fragmentos de Lua que leen sensores se ejecutan dentro de `step()`; no pegarlos fuera de las funciones del controlador. Los fragmentos XML son partes de un experimento: los archivos completos están en [examples/](../examples).

## Referencia rápida

| Función | XML del controlador | Lua | C++ |
|---|---|---|---|
| Ruedas | actuador `differential_steering`, `default` | `robot.wheels.set_velocity(l,r)` | `CCI_DifferentialSteeringActuator::SetLinearVelocity` |
| Encoder | sensor `miniqupa_encoder`, `default` | `robot.wheels.encoder` | `CCI_MiniQupaEncoderSensor::GetReading` |
| IR | sensor `miniqupa_ir`, `default` | `robot.miniqupa_ir` | `CCI_MiniQupaIRSensor::GetReadings` |
| Color de suelo | sensor `miniqupa_floor_color`, `default` | `robot.floor_color` | `CCI_MiniQupaFloorColorSensor::GetReading` |
| Difusor RGB | actuador `miniqupa_diffuser`, `default` | `robot.diffuser.set_color(r,g,b)` | `CCI_MiniQupaDiffuserActuator::SetColor`, `TurnOff` |
| Blobs | sensor `miniqupa_camera`, `default` | `robot.camera` | `CCI_MiniQupaCameraSensor::GetBlobs`, `GetRawSampleCount` |
| Radio | actuador y sensor `simple_radios`, `default` | `robot.simple_radios.espnow` | `CCI_SimpleRadiosActuator/Sensor::GetInterfaces` |
| RAB | actuador `range_and_bearing`, `default`; sensor `medium` | `robot.range_and_bearing` | `CCI_RangeAndBearingActuator/Sensor` |
| Tag | atributo de entidad `tag_id` | Sin API propia del controlador MiniQupa | Configuración de escenario; detector de otro robot |

## Convenciones y glosario

- **Entidad**: el robot `<miniqupa>` y sus componentes físicos/visuales.
- **Controlador**: el programa que lee sensores y ordena actuadores.
- **Medio**: infraestructura compartida para LED, radio, RAB o tags; no es un sensor adicional.
- **Blob**: segmento de color observado en un azimut, no una identidad de robot.
- **Referencia local**: X delante, Y izquierda, Z arriba. Origen al nivel del suelo bajo el eje de ruedas.
- **Paso de control**: una llamada a `step()` o `ControlStep()`. En los ejemplos, 20 Hz, es decir 0,05 s. Las iteraciones internas de dynamics2d no son llamadas adicionales al controlador.
- XML de posiciones y alcances: **metros**; orientación del cuerpo y `tag_yaw`: **grados**.
- Velocidad de ruedas y distancia recibida por RAB: **cm/s** y **cm**, por la API genérica ARGoS.
- Encoder: **metros por muestra**; ángulos de sensores: **radianes**.

No hay IMU, control de LEDs inferiores, feromonas, imágenes RGB de cámara ni simulación completa de ESP-NOW. El encoder y la óptica son idealizaciones. Consulta los límites de cada capítulo antes de trasladar un algoritmo al robot físico.
