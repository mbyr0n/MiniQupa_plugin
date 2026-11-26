# Comunicación: simple radios y range-and-bearing

[Índice](README.md)

Son dispositivos independientes. Simple radios intercambia contenido; RAB añade distancia y ángulos relativos. La radio no proporciona por sí sola una medida RAB ni una identidad MAC de ESP-NOW.

## Simple radios

```xml
<!-- Entidad: -->
<miniqupa id="mq0" radio_medium="radios" radio_range="1" radio_position="0,0,0">
  <body position="0,0,0" orientation="0,0,0"/>
  <controller config="smoke"/>
</miniqupa>
<!-- Actuators y sensors: uno en cada bloque -->
<simple_radios implementation="default"/>
<!-- Media: -->
<simple_radio id="radios" grid_size="10,10,5"/>
```

La interfaz se llama `espnow`, pero usa **la API nativa simple radios de ARGoS**. `radio_range` está en metros (inicial 1 m); `radio_position` es una referencia local virtual, no una antena medida. Si omites `radio_medium` no se crea esa interfaz de radio para emitir/recibir.

### Lua: enviar y recibir

```lua
-- En step(), puede ser una tabla con un protocolo sencillo propio:
robot.simple_radios.espnow.send({42, 7, 125})
for _, mensaje in ipairs(robot.simple_radios.espnow.recv) do
   local emisor_logico = mensaje[1]
   local tipo = mensaje[2]
   local valor = mensaje[3]
   log("emisor=" .. emisor_logico .. " tipo=" .. tipo .. " valor=" .. valor)
end
```

`send(tabla)` serializa una tabla Lua. El ejemplo usa enteros 0…255 para acercarse a un protocolo de bytes; **no significa que cada número Lua ocupe un byte en la serialización de ARGoS**. `recv` contiene las tablas deserializadas disponibles en la muestra actual. Puede estar vacío. Procesa sus mensajes una vez por paso si no quieres duplicar efectos en tu algoritmo.

No se añade automáticamente un emisor, destinatario, secuencia o checksum al contenido recibido. Si quieres direccionamiento, acuerda campos, por ejemplo `{origen, destino, secuencia, valor}` y filtra al recibir; eso es un protocolo de aplicación, no una nueva propiedad física de la radio. Todos los receptores habilitados que cumplan las condiciones del modelo pueden recibir el envío. No esperes entrega dentro de la misma llamada `step()` en que se emitió: se observa en muestras posteriores.

El plugin no incluye una capa completa ESP-NOW: no reproduce MAC, emparejamiento, canales Wi-Fi, confirmación real de entrega, RSSI ni el límite de 250 bytes del hardware. La capa opcional del workspace de desarrollo **no forma parte de este repositorio**. Aquí se documenta exactamente la API nativa incluida.

### C++: datos binarios

```cpp
#include <argos3/plugins/robots/generic/control_interface/ci_simple_radios_actuator.h>
#include <argos3/plugins/robots/generic/control_interface/ci_simple_radios_sensor.h>
auto* tx = GetActuator<argos::CCI_SimpleRadiosActuator>("simple_radios");
auto* rx = GetSensor<argos::CCI_SimpleRadiosSensor>("simple_radios");
for(auto& interfaz : tx->GetInterfaces()) {
   if(interfaz.Id == "espnow") {
      argos::CByteArray bytes;
      bytes << argos::UInt8(42) << argos::UInt8(7);
      interfaz.Messages.push_back(bytes);
   }
}
for(const auto& interfaz : rx->GetInterfaces()) {
   for(const auto& mensaje : interfaz.Messages) {
      // Para este protocolo binario propio:
      if(mensaje.Size() == 2) { auto emisor = mensaje[0]; }
   }
}
```

`Messages` es la cola de mensajes a transmitir o la colección recibida. La implementación del simulador procesa/limpia sus colas entre pasos; no hagas crecer un historial manual dentro de la cola del actuador. **El CByteArray binario de este fragmento no equivale a la tabla serializada de Lua**. Si mezclas emisores Lua y C++, utiliza explícitamente la serialización/deserialización de tablas de ARGoS o un protocolo común; no indexes un paquete Lua como si fuera tu vector de bytes binario.

## Range and bearing

```xml
<!-- actuators -->
<range_and_bearing implementation="default"/>
<!-- sensors -->
<range_and_bearing implementation="medium" medium="rab"/>
<!-- media -->
<range_and_bearing id="rab" grid_size="10,10,5"/>
```

Atributos de entidad: `rab_range="3"` en metros, `rab_data_size="10"` bytes y `rab_position="0,0,0.1"` en metros. Tamaño y rango deben ser positivos. Configura tamaños compatibles entre robots; los ejemplos usan diez bytes para todos.

```lua
-- Sustituir todo el contenido: exactamente rab_data_size entradas.
robot.range_and_bearing.set_data({42, 1, 2, 3, 0, 0, 0, 0, 0, 255})
-- O cambiar un solo byte; índices Lua empiezan en 1.
robot.range_and_bearing.set_data(2, 9)
-- Poner todos los bytes a cero.
robot.range_and_bearing.clear_data()
```

Los valores deben ser enteros entre 0 y 255; valida tu protocolo antes de enviarlos. La API nativa puede convertir números a UInt8 y no debe usarse esa conversión como validación. `clear_data()` no apaga el emisor: sigue existiendo un paquete de datos a cero. El contenido persiste hasta otra modificación o reset.

```lua
for _, paquete in ipairs(robot.range_and_bearing) do
   local distancia_m = paquete.range / 100
   local azimut = paquete.horizontal_bearing
   local elevacion = paquete.vertical_bearing
   local emisor_logico = paquete.data[1]
end
```

| Lua | C++ `SPacket` | Unidad |
|---|---|---|
| `range` | `Range` | **Centímetros**, convertir a m dividiendo por 100 |
| `horizontal_bearing` | `HorizontalBearing` | Radianes, relativo al receptor |
| `vertical_bearing` | `VerticalBearing` | Radianes |
| `data` | `Data` | Bytes; índices Lua 1…N, C++ 0…N−1 |

No confundas `rab_range` del XML (m) con `paquete.range` (cm). La distancia corresponde a las referencias RAB virtuales, no al espacio entre carcasas. Los paquetes no exponen un ID único nativo del emisor; envía uno en `data` si lo necesitas. No supongas un orden fijo de vecinos en la lista.

### C++

```cpp
#include <argos3/plugins/robots/generic/control_interface/ci_range_and_bearing_actuator.h>
#include <argos3/plugins/robots/generic/control_interface/ci_range_and_bearing_sensor.h>
auto* tx = GetActuator<argos::CCI_RangeAndBearingActuator>("range_and_bearing");
auto* rx = GetSensor<argos::CCI_RangeAndBearingSensor>("range_and_bearing");
argos::CByteArray datos(tx->GetSize()); // Inicializa el paquete completo.
datos.Zero(); datos[0] = 42;
tx->SetData(datos);
tx->SetData(1, 9); // Segundo byte: índice C++ empieza en 0.
for(const auto& paquete : rx->GetReadings()) {
   const auto metros = paquete.Range / 100.0;
}
tx->ClearData();
```

RAB reutiliza la implementación nativa ARGoS; no es una medida física derivada de ESP-NOW. Las opciones de ruido/oclusión del sensor nativo dependen de la versión ARGoS. Este manual verifica los valores por defecto de beta59; no promete fidelidad de radiofrecuencia.

El [ejemplo de dos robots](../examples/neighbors.lua) verifica datos y distancia de 30 cm, con ambos cuerpos quietos y sus referencias a la misma altura.
