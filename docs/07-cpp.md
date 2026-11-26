# Controladores C++

[Índice](README.md)

Los ejemplos Lua y C++ consumen las mismas interfaces; no es necesario modificar los fuentes del robot para cambiar el comportamiento. El ejemplo compilable está en [examples/cpp/](../examples/cpp).

## Compilar el controlador de ejemplo

Primero compila e instala el plugin según el README. Desde la raíz del repositorio:

```bash
cmake -S examples/cpp -B build-controller -DMINIQUPA_PREFIX="$PWD/install"
cmake --build build-controller -j4
```

Para una instalación personalizada ARGoS añade el mismo `-DCMAKE_PREFIX_PATH` usado al compilar el robot. El controlador se enlaza contra la biblioteca instalada y utiliza sus headers públicos; no depende del árbol fuente ARGoS ni de rutas de desarrollo.

Exporta las variables del [primer capítulo](01-primer-controlador.md) y añade la biblioteca del controlador:

```bash
export ARGOS_PLUGIN_PATH="$PWD/build-controller:$ARGOS_PLUGIN_PATH"
export LD_LIBRARY_PATH="$PWD/build-controller:$LD_LIBRARY_PATH"
argos3 -c examples/cpp.argos -z
```

Al final debe aparecer `MINIQUPA_CPP_OK`. El ejemplo mueve las ruedas, acumula encoder, lee IR, suelo, blobs y mensajes, y envía datos binarios por radio y RAB. Solo hay un robot en esta escena, así que sus colecciones de vecinos deben estar vacías. El ejemplo Lua de vecinos comprueba recepción real entre dos robots.

## Ciclo de vida

```cpp
class CMyController : public argos::CCI_Controller {
 public:
  void Init(argos::TConfigurationNode& tree) override;
  void ControlStep() override;
  void Reset() override;
  void Destroy() override;
};
REGISTER_CONTROLLER(CMyController, "mi_controlador");
```

`Init()` obtiene los dispositivos mediante `GetSensor<T>("nombre_xml")` y `GetActuator<T>("nombre_xml")`. `ControlStep()` procesa las lecturas y da órdenes. `Reset()` reinicia el estado propio. `Destroy()` libera recursos. Los punteros de dispositivos pertenecen a ARGoS: no hagas `delete` sobre ellos.

La etiqueta de `REGISTER_CONTROLLER` es el nombre del elemento bajo `<controllers>`; el atributo `id` de ese elemento es la referencia usada por la entidad. El atributo `library` debe resolver a la biblioteca compilada del controlador.

## Headers y acceso

Los propios están bajo `argos3/plugins/robots/miniqupa/control_interface/`:

| Header | Clase y acceso |
|---|---|
| `ci_miniqupa_ir_sensor.h` | `CCI_MiniQupaIRSensor`, `GetReadings()` |
| `ci_miniqupa_encoder_sensor.h` | `CCI_MiniQupaEncoderSensor`, `GetReading()` |
| `ci_miniqupa_floor_color_sensor.h` | `CCI_MiniQupaFloorColorSensor`, `GetReading()` |
| `ci_miniqupa_camera_sensor.h` | `CCI_MiniQupaCameraSensor`, `GetBlobs()`, `GetRawSampleCount()` |
| `ci_miniqupa_diffuser_actuator.h` | `CCI_MiniQupaDiffuserActuator`, `SetColor()`, `TurnOff()` |

Los genéricos están bajo `argos3/plugins/robots/generic/control_interface/`: `ci_differential_steering_actuator.h`, `ci_simple_radios_actuator.h`, `ci_simple_radios_sensor.h`, `ci_range_and_bearing_actuator.h` y `ci_range_and_bearing_sensor.h`.

Los ángulos propios son `CRadians`: usa `.GetValue()` para un número en radianes. Colores: `CColor` con `GetRed()`, `GetGreen()` y `GetBlue()`. Arrays RAB y vectores C++ comienzan en índice 0, a diferencia de Lua. Las lecturas referenciadas con `const auto&` son la muestra actual; cópialas si quieres conservar un historial.

Consulta los campos/unidades en los capítulos de cada dispositivo. Un nombre que aparezca en la clase entidad no implica una API del controlador: usa las CI públicas. El tag superior se configura por XML; no se obtiene con `GetSensor()`.
